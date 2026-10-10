# The engine: a map for modders

The card scripts (s044..s135, see [CARDS.md](CARDS.md)) tell the story. Everything else is the
**engine**: the resident root, the start-up overlay s000, the battlefield generator and the
tactical combat (s002..s028), the strategic layer (s029..s041) and the card engine s043. This
page lists what each part does, its key functions and data under their readable names, how
far its C has got, and how to change it.

## Reading the engine sources

Engine C lives in two trees:

| tree | what it is | used by |
|---|---|---|
| `src/c/root`, `src/c/s000..s043` | **byte-exact** C: every function compiles to the original bytes (`tools/fmatch.py` says FILE MATCH) | every build (`tools/build.py`) |
| `src/wip/**` | **drafts**: the whole module written in C, matching only partly (the rest is functionally equivalent, written from the disassembly and play-tested) | `tools/build.py --mod --functional` (stamped drafts not listed in `src/wip/EXCLUDE`) and as a starting point for mods |

The decompiled C uses raw linker names: `D_xxxx` (a DGROUP offset), `F_ssss_xxxx` (far data
at segment ssss), `sub_<image>_<frame>_<offset>` (an original function) and `vec_xxxx` (an
RTLink overlay call vector, the way one overlay calls a function of another). Two headers give
them readable names:

* `include/darklands/card.h`: the card-script API (party, leader, calendar, money, locations,
  events, the s029 rules, the s043 card engine). See CARDS.md.
* `include/darklands/game.h`: the rest of the engine (about 600 names), grouped by module.

Both hold `#define readable RAW` lines only, so a file written with the readable names compiles
to the same object code. Engine files include both (`card.h` first). A function and its
overlay vector share a name: the vector is `<name>_vec` (`door_set` / `door_set_vec`), or, when
card.h already names the vector, the function is `<name>_impl` (`add_item` = vec_2067,
`add_item_impl` = sub_s031_18e7_0128). A comment starting with `?` marks a name whose meaning
is not fully certain. Some functions also have a symbolic linker name in a few sources
(`alloc_buffer`, `random_n`, `copy_rect_saved`, `party_info`, `saint_screen`...): those stay
as they are, and game.h gives the raw name a different readable name (`mem_alloc`,
`rand_n`, `rect_copy_paged`, `party_info_screen`).

    python3 -I tools/gamenames.py FILE.c            # convert an engine file in place
    python3 -I tools/gamenames.py --check FILE...   # would anything change?
    python3 -I tools/gamenames.py --diff FILE.c     # preview
    python3 -I tools/gamenames.py --raw FILE.c      # raw names still without a readable name

`gamenames.py` works like `cardnames.py` (which stays the tool for the card scripts): it
renames in code and comments, never inside string or character literals, never inside
`assert(...)` arguments or `/* compat600-off: */` / `/* empty-ok: */` directives, never
moves a declaration, adds the two `#include` lines under the leading comment block, and is
idempotent. It refuses a file that already uses a readable name for something else (a
local, a parameter, a struct member) or `#define`s a readable name. A raw name that a file
`#define`s itself (`#define F_20a5_0000 (F_20a5_000.cell)`) keeps its raw spelling in that file.

**Always re-run `tools/fmatch.py` after converting or after editing game.h.** The macros
cannot change what the compiler sees, but MSC 6.00A's symbol table can: including game.h
enters every raw name it mentions into the compiler's name table in header order, and some
files' register/operand choices depend on that order (see "symbol-table state" in
PORTING_NOTES.md). `src/wip/s014/s014_12f7.c` is such a file: including game.h alone turns
13/18 matching functions into 11/18, so it keeps its raw names and carries a
`/* gamenames: off */` comment, which makes the tool refuse it. To check a batch of
conversions, save `python3 -I tools/fmatch.py -v FILE` before, convert, and compare the output
after: it must be identical (and the same after any later edit of game.h).

## Memory and build rules (what a mod may do)

* **DGROUP is full.** A module's string literals and initialised data (`_DATA`, `CONST`) may
  shrink but not grow. New text goes into the code segment:
  `static char _based(_segname("_CODE")) s_new[] = "...";`, used as `(char far *)s_new`
  (copied with `_fstrcpy` or passed as `char far *`). This takes no DGROUP space. New
  variables: a `static` in the code segment the same way (read/write through a far pointer).
* **Root code cannot grow.** The resident root is not re-laid out: a root draft that is
  longer than the original is skipped by `--functional` (and a root mod must fit). Overlay
  code may grow: `--mod` re-lays out the overlay tree around the larger code.
* **Only existing globals and functions.** Declare what you use at the top of the file
  (`extern struct cbt combatants[];`, `int far rand_mod();`). New globals: code-segment statics (see above).
* **Overlays are siblings.** A function of another overlay is called through its vector
  (`*_vec`, or the card.h vector names); calling `sub_...` of a sibling overlay directly only
  works when that overlay is a parent of yours (s029 is the parent of s030..s043, s014 of
  s015..s028, s004 of s005..s013).

Build a mod:

1. Copy the file you want to change (from `src/c`, or the draft from `src/wip`) into a mod
   directory, e.g. `mods/my_mod/s036/s036_1462.c`. Keep the `/* place: */`, `/* orig: */`
   and `/* flags: */` lines: they say which original code the file replaces.
2. Edit it, with the readable names (the includes resolve from `include/`).
3. Build: `python3 -I tools/build.py <DARKLAND.EXE> --skip-analysis --mod --overlay mods/my_mod --out my.exe`
   (add `--functional` to use every stamped `src/wip` draft as well, i.e. the whole engine
   in C). See `mods/` for working examples: `cheap_inn` (s036 inn prices), `heavy_blows`
   (s014 damage), `strong_party` (s001 default party), `swift_travel` (s030 overland speed),
   `rich_start` (a card script).
4. A mod build rewrites `build/symbols.json`: run a normal build afterwards (or give the mod
   its own `--work`), or `fmatch.py` reports nonsense.

## Core data (card.h and game.h names)

| data | name | layout |
|---|---|---|
| party members | `party` (D_9c69), `party_size`, `leader`, `party_seed` | 5 records of 0x80 bytes: +0 status (1 able), +0x2a name, +0x35 fatigue, +0x37.. armour/weapon types and qualities, +0x49 attributes (`member_attrs`, 0 End 1 Str 2 Agl 3 Per 4 Int 5 Chr 6 DF), +0x50 their maxima, +0x57 skills (0-6 weapons, 7 Alchemy, 8 Religion, 9 Virtue, 10 Latin, 11 Reading, 12 Healing, 13 Artifice, 14 Stealth, 15 Streetwise, 16 Riding, 17 Woodwise), +0x6a item count. `who` = 0..4, -1 leader, -2 everybody |
| inventories | `inventories` (F_2e38_003e), `item_defs` | per member 0x180 bytes of 6-byte records {code, type, quality, count, weight}; definitions from darkland.lst |
| knowledge | `saints_known` (0x14 bytes per member, names `saint_names`), `formulas_known` (0x16 bytes, `virtue_names`) | bit sets |
| money | `party_money` (`party_money_gr`, `party_money_pf`) | struct money {florins, groschen, pfennigs}; 1 fl = 20 gr, 1 gr = 12 pf |
| time | `cur_hour`, `cur_day`, `cur_month`, `cur_year`, `days_per_month` | day and month 0-based |
| world | `locations` (0x19e far records, cities 0..0x5b), `cur_loc`, `city_rec`, `city_desc`, `party_x`/`party_y`, `party_fame` | location: type, x, y, ..., reputation, flags |
| timed events | `events` (300 far records of 0x30 bytes) | type0, start and end date, owner f1a (-2 party), location f1c, value f1e, kind +0x22, f28..f2e; durations in hours, 9999 = forever |
| options | `game_difficulty` (0 basic, 1 standard, 2 expert), `opt_sound`, `opt_music`, `opt_saint_msgs` | |
| screen | `screen_win` (visible page), `back_win` (off-screen page), `card_window`, `cur_palette`, `mouse_x`/`mouse_y`/`mouse_buttons` | struct gfx: +0x0c colour, +0x0e background, +0x10 font |
| combatants | `combatants` (D_9c55) | 13 records of 0x80 bytes: 0..4 the party (`party` is `combatants` + 0x14), 5..12 enemies; +0 screen x/y, +4 map x/y, +8 destination, +0x10 action, +0x14 status (1 active, 2 unconscious, 3 dead, 5 left), +0x16 animation slot, +0x4a target, +0x5d endurance, +0x5e strength. Parallel arrays `cbt_floor`, `cbt_action`, `cbt_anim`, `cbt_flags`, `cbt_attack_rate`, `cbt_queued_damage` ... |
| battlefield | `floor_img` (F_20a5_0000, 0x33fc bytes = level%d.flr) | cells (long [40][40], column-major, 32 x 16 pixels: wall nibbles, furniture, bit 31 impassable), heights at +0x1900, door records +0x1f5a, triggers +0x2184, wall groups +0x248a, exits +0x2fb8, chests +0x3014, secret doors +0x3350, `floor_doors` +0x33d3 |

## Modules

Status: **exact** = `src/c`, byte-identical; **draft n/m** = `src/wip`, n of m functions
match, the rest functionally equivalent. The full per-file list is at the end.

### root (resident): shell, graphics, files, memory

* **root 0000** (`src/wip/root/root_0000.c`, draft 27/38): the game shell. Menu bar
  (`menu_bar_run`, `menu_dropdown_open`), hot keys (`hot_keys`: Alt-D difficulty, Alt-F sound,
  Alt-M music, Alt-C saintly changes), options (`toggle_sound`, `toggle_music`,
  `toggle_saint_msgs`, `set_difficulty_*`, `show_difficulty`), random numbers (`rand_mod`
  0..n-1, `rand_1_n`), `quit_to_dos`, `game_shutdown`, message boxes (`show_message`,
  `ask_yes_no`, `text_box`, `draw_frame_box`), the party panel (`party_panel_frames`,
  `party_panel_rows`, `member_bars`), the current data file (`datafile_select`).
* **root 0265** (`src/wip/root/text.c`, draft 8/10): text output: `draw_text`,
  `draw_text_far`, `draw_text_centered`, `draw_text_wrapped` (control codes, hot spots),
  `measure_text_wrapped`, `text_pixel_width`, `ltoa_commas`.
* **root 030b** (`src/c/root/root_030b.c`, exact): the PAN animation player ("player.c"),
  streaming cels through a DOS ring buffer (FAB stream: `fab_open`, `fab_read`, `fab_close`).
* **root 03ec** (`src/c/root/rectsave.c`, exact): `rect_copy_paged` (copy_rect_saved).
* **root 0410** (`src/c/root/random.c`, exact; the other helpers are assembly): `rand_n`,
  `dir_add`/`dir_sub`, `sign_of`, `word_fill`, `index_of_*`, `palette_save`/`palette_restore`.
* **root 047c** (`src/wip/root/root_047c.c`, NOT C: hand-written assembly, documented only):
  the interface to the graphics driver MGRAPHIC.EXE (self-patched far-jump thunks), timer,
  palette cycling, mouse, keyboard. Names for the thunks whose use is clear:
  `gfx_copy_rect`, `gfx_text`, `gfx_draw_pic`, `gfx_draw_sprite`, `gfx_fill_rect`,
  `gfx_line`, `gfx_recolor_rect`, `gfx_save_rect`, `set_palette`, `char_width`,
  `key_pressed`, `get_key`, `sound_play`, `distance3`, `mouse_show`/`mouse_hide`,
  `mouse_set_cursor`. Not moddable as C.
* **root 05ba / 044a** (`src/c/root/membuf.c`, exact): DOS memory: `mem_alloc`
  (alloc_buffer), `mem_free`, `mem_realloc`; DOS wrappers `dos_alloc`, `dos_free`,
  `dos_resize`, `dos_print`.
* **root 05f8**: EMS driver interface (`ems_map_page`, used with `ems_frame_handle`).
* **root 0617** (`src/wip/root/fileio.c`, draft 8/29, excluded from `--functional`: built
  with an older compiler): `file_open` (reads from the open .CAT catalogue when it has the
  file), `file_load`, `file_load_far`, `file_save`, `catalog_open`/`catalog_find`, pictures
  (`pic_load`, `pic_draw`, `pic_draw_pal`, `pic_draw_mem`), `font_load`.
* **root 06a1 / 09c0 / 0d14**: the Microsoft C library and the RTLink/Plus overlay manager
  and cache (assembly, `symbols/names.txt`).

Root mods: root code cannot grow and DGROUP is full, so keep root changes the same size or
smaller; prefer to change the overlay that calls the root function.

### s000, s001: start-up and the default party

* **s000** (`src/wip/s000/s000_0e76.c` draft 8/11, `src/c/s000/s000_0fb2.c` exact): `main`,
  `startup_config` (title screen, config.drk, command line), EMS/XMS checks,
  `battle_menus_enable`, video mode detection/restore (`video_detect`, `video_restore`).
* **s001** (`src/wip/s001/s001_0e76.c`): `party_default` builds the four pre-generated
  characters (names, attributes, skills, equipment). See `mods/strong_party`.

### s002..s013: battles and battlefield construction

* **s002** (draft, not converted: it uses `party` as a local): `battle_run` (= vec `battle_run_vec`) sets up one
  battle (mode, battlefield type, seed, up to three enemy groups), loads tiles
  (`battle_tiles_load`) and portraits, runs combat and reports the result in `battle_result`.
* **s003** (draft 4/8): battle save files (saves\<name>.bsv, PKWARE implode/explode).
* **s004** (drafts; `s004_1212/1223/1237` exact copies of the s026 swap code):
  `battlefield_build` loads darkland.enm (enemy types `enemy_types`, monster templates,
  `tile_desc`) and runs the generator overlays; `map_turn` turns/mirrors prepared maps;
  monster/group fix-ups (`bf_monsters`, `bf_groups`, `mon_count`, `group_count`); `roll_dice`.
* **s005/s006/s007** (drafts): the procedural castle/keep generator. Work area `gen_work`
  (rooms, connections, cells: `gen_cell_type`, `gen_cell_walls`, `gen_cell_door`, exits
  `gen_exits`), keep and towers (`keep_x0`, `gen_strips`), `gen_add_room`, corridors, stairs;
  s007 `castle_build` turns it into level buffers and furnishes rooms (tables, thrones, chests).
* **s008** wilderness (`wild_build`, prepared maps iwild*), **s009** mines (`mine_build`,
  sections iminegen/iminspec), **s010** cities (`city_build`, `city_battle_type`), **s011**
  Templar fortress (`templar_build`), **s012** dragon cave, **s013** Baphomet's citadel.
  All drafts; they write `level%d.flr` (the `floor_img` layout) and the enemy files.

### s014..s028: tactical combat

* **s014** (drafts): the battle. `combat_main` (entry and loop: `combat_tick` every 8 clock
  ticks: AI `ai_tick`, `party_locations`, `melee_exchanges`, `combat_actions`, `combat_draw`);
  `combat_over` (result in `battle_result`: 0 enemies beaten, 2 fled, 3 party lost, 5 all
  down ...). Pathfinder (`path_find`, bug-algorithm trackers, s014_12f7.c: not converted, see
  above), collisions (`cbt_collisions`), renderer (`combat_draw`, three sprite lists, dirty
  tiles), attacks (`cbt_attack`, `to_hit_roll`, `hit_damage`, `damage_queue`,
  `damage_apply`: endurance 0 = unconscious, strength 0 = dead; `weapon_ready`, `use_ammo`),
  user interface (`combat_mouse`, order mode `order_mode_enter`/`order_mode_leave`,
  `selected_member`, `order_action`, loot piles `loot_pile_open`, `loot_after_fight`),
  missiles (`missiles_advance`), line of sight (`line_blocked`, `los_x0..los_y1`), AI
  (`ai_set_target`, `ai_rethink`). Mod example: `mods/heavy_blows` (s014_18bf damage).
* **s015** debug screens, **s016** combat keys (`combat_keys`, `member_activate`, marching
  order `marching_order_rebuild`, `single_file`; draft 56/59), **s017** floor view
  (`floor_view_draw`), **s018** missiles and enemy AI (`missile_launch`, `enemy_turn`,
  `enemy_sees`), **s019** spell/effect areas (exact), **s020** level loading, **s021** skill
  improvement in combat and enemy info (exact: `skill_try_improve`, `skill_change_msg`,
  `enemy_info`), **s022** character sheet, `do_surrender`, party order, **s023** doors,
  locks, traps, chests, stairs (`door_toggle`, `door_set`, `lock_pick`, `chest_open`,
  `chest_disarm`, `trap_spell`, `trap_missile`, `take_exit`), **s024** level switching
  (`level_switch`, `furniture_load`, `place_around`), **s025** enemy placement and
  animation buffers (`enemies_place`, `enemy_anim_load`).
* **s026..s028**: the swap stores (XMS, EMS, temporary files) used to keep big buffers:
  `xms_read`/`xms_write`, `ems_read`/`ems_write`, `swapfile_read`/`swapfile_write` (s026,
  exact), named blocks `swap_save`, `swap_load`, `swap_load_free` (s027, draft 17/21),
  store set-up `swap_init`, `ems_present`, `xms_present` (s028, draft 33/36). Globals
  `xms_entry`, `xms_version`, `ems_frame`, `ems_frame_handle`, `swap_stores`.

### s029..s041: the strategic layer

* **s029** rules core: `src/c/s029/s029_1367.c` (exact: clamps, money, calendar,
  `date_*`, `days_since`, `chance_text`, `random_name`) and `src/wip/s029/s029_0e76.c`
  (draft 136/143: party member accessors, party queries, items, saints, `run_battle`,
  `hours_pass`, locations `loc_query`/`loc_set`, timed events `event_*`). Most of its API
  has card.h names (CARDS.md).
* **s030** overland map (draft 16/22): `overland_map`, `overland_loop`, `nearest_city`,
  `map_message`. Mod example: `mods/swift_travel`.
* **s031** inventory screen (draft 18/19: `member_sheet`) and inventory helpers (draft 25/27:
  `add_item_impl`, `take_item_impl`, `give_one`, `item_move`, `equip_weight`...).
* **s032** barter (draft 8/11): `barter_screen`, `buy_price`, `sell_price`, `shop_has_item`.
* **s033** alchemy: screen (draft 9/16, `alchemy_screen`) and formula viewer (exact,
  `alc_view`).
* **s034** saints / prayer screen (`saints.c`, exact, not converted: it uses `party` as a
  parameter) and the party information screen (`partyinfo.c`, exact, `party_info_screen`).
* **s035** item cache (exact, `item_cache_screen`). **s036** camping, inns, day labour (draft
  19/23, `camp_spend_day`; mod example `mods/cheap_inn`).
* **s037/s038** character creation, start screen, hall of fame, save and load
  (`save_game`, `load_game`, `load_saved_game`, `party_add`, `party_remove`,
  `create_character`, `text_input`; drafts and exact files).
* **s039** the event-rules toolkit the card scripts call through vectors (exact:
  `practice_skill_impl`, `injure_impl`, `give_reward_impl` ...) and the 138 saint functions
  (`s039_165c.c`, exact, called through `saint_fns`).
* **s040** loot distribution (draft 9/10, `loot_distribute`). **s041** copy protection
  (exact, `copyprot_run`).

### s043: the card engine

`src/c/s043/s043_1462.c` (exact: `world_load`, `cards_startup`, `cards_shutdown`, the shared
card strings), `s043_17e5.c` (exact: `palette_fade`) and `src/wip/s043/s043_150b.c` (draft
24/28): `card_load`, `msg_load` (msgfiles\<card>.msg into `msg_file`, `msg_blocks`,
`msg_index`), `text_expand` / `var_expand` ($variables, `var_names`), the option menu
`card_menu` with `card_mouse` and `highlight_rect`, `card_global_keys` (F1..F5 sheets),
`pass_hours_hourglass`. The script side is in CARDS.md.

## Known bugs of the original game

Recorded by the porting agents in the source comments (kept in the C, since the matching
code must reproduce them; a mod may fix them):

* **s029 0e76** (`src/wip/s029/s029_0e76.c`):
  - `member_best_skill` (01c0) sets `best_skill` for every skill (missing braces), so it ends as 6.
  - `set_attr_max` (0b84) computes the 0..99 clamp of the new value and stores the unclamped value.
  - 1696 (lowest attribute sum a + b) and `party_max_attr_sum` (179c) store 2*a instead of a + b.
  - `improve_roll` (18a8) discards the clamp of v.
  - `count_items` (0c76) for the store lists (who >= 5) reads the quantity from the party
    inventories instead of from the list.
  - `loc_set` (1dfe) what 37: instead of shifting the three-slot list, every occupied slot is
    overwritten.
  - `run_battle` (2278): the location seed gets `party_seed` added a second time.
  - `lower_skill_temp` (4b08): the -2 (whole party) loop queries and changes member -2
    instead of each member, and the capped change carries over.
  - `raise_attr_temp` (4c04): in the -2 loop a member whose gap is smaller than d ends the
    whole function, and the reduced d carries over to the next member.
  - `event_value` (39f0) reads event -1 when no event exists; 4db6 / `event_48_max` (4e0c)
    return garbage when the scan reaches slot 300.
* **s039 1462** 1e90 (pay for a service): sets the money to cost - money (sic); an unknown size
  uses an uninitialised factor.
* **s038 16cd** 0404: merges the party into the hall of fame table but never writes it back.
* **s037 1462** 0444: the second colour case's fifth "A00" test cycles the wrong colour
  (D_e894).
* **s032 barter** 1d18/078e: the shop size is read from an uninitialised stack slot (the
  previous call's clamped value), so from the second call on shops use a smaller stock table
  (the draft keeps this with a `volatile` local).
* **s014 19cd** (draft history): behaviour bugs found and fixed in the drafts, not in the game,
  are listed in QA.md.

## Per-file status (after the game.h conversion)

| file | place | tree | fmatch | converted to game.h |
|---|---|---|---|---|
| `src/c/root/membuf.c` | root 05ba | exact | 3/3 FILE MATCH | yes |
| `src/c/root/random.c` | root 0410 | exact | 1/29 | nothing to rename |
| `src/c/root/rectsave.c` | root 03ec | exact | 1/1 FILE MATCH | yes |
| `src/c/root/root_030b.c` | root 030b | exact | 20/20 FILE MATCH | yes |
| `src/c/s000/s000_0fb2.c` | s000 0fb2 | exact | 2/2 FILE MATCH | yes |
| `src/c/s004/s004_1212.c` | s004 1212 | exact | 2/2 FILE MATCH | yes |
| `src/c/s004/s004_1223.c` | s004 1223 | exact | 2/2 FILE MATCH | nothing to rename |
| `src/c/s004/s004_1237.c` | s004 1237 | exact | 4/4 FILE MATCH | yes |
| `src/c/s018/s018_1ef8.c` | s018 1ef8 | exact | 4/4 FILE MATCH | yes |
| `src/c/s019/s019_1ef8.c` | s019 1ef8 | exact | 17/17 FILE MATCH | yes |
| `src/c/s021/s021_1ef8.c` | s021 1ef8 | exact | 5/5 FILE MATCH | yes |
| `src/c/s026/s026_1ef8.c` | s026 1ef8 | exact | 2/2 FILE MATCH | yes |
| `src/c/s026/s026_1f09.c` | s026 1f09 | exact | 2/2 FILE MATCH | yes |
| `src/c/s026/s026_1f1d.c` | s026 1f1d | exact | 4/4 FILE MATCH | yes |
| `src/c/s029/s029_1367.c` | s029 1367 | exact | 42/42 FILE MATCH | yes |
| `src/c/s030/sysutil.c` | s030 1767 | exact | 7/7 FILE MATCH | nothing to rename |
| `src/c/s033/s033_15d1.c` | s033 15d1 | exact | 8/8 FILE MATCH | yes |
| `src/c/s034/partyinfo.c` | s034 1550 | exact | 9/9 FILE MATCH | yes |
| `src/c/s034/saints.c` | s034 1462 | exact | 12/12 FILE MATCH | no (uses `party` as a local) |
| `src/c/s035/cache.c` | s035 1462 | exact | 11/11 FILE MATCH | yes |
| `src/c/s037/s037_1c7d.c` | s037 1c7d | exact | 11/11 FILE MATCH | yes |
| `src/c/s037/s037_1e26.c` | s037 1e26 | exact | 5/5 FILE MATCH | yes |
| `src/c/s038/s038_16cd.c` | s038 16cd | exact | 5/5 FILE MATCH | yes |
| `src/c/s038/s038_172a.c` | s038 172a | exact | 4/4 FILE MATCH | yes |
| `src/c/s039/s039_1462.c` | s039 1462 | exact | 43/43 FILE MATCH | yes |
| `src/c/s039/s039_165c.c` | s039 165c | exact | 139/139 FILE MATCH | yes |
| `src/c/s041/copyprot.c` | s041 1462 | exact | 4/4 FILE MATCH | yes |
| `src/c/s043/s043_1462.c` | s043 1462 | exact | 6/6 FILE MATCH | yes |
| `src/c/s043/s043_17e5.c` | s043 17e5 | exact | 3/3 FILE MATCH | yes |
| `src/wip/root/fileio.c` | root 0617 | draft (EXCLUDE) | 8/29 | yes |
| `src/wip/root/root_0000.c` | root 0000 | draft (--functional) | 27/38 | yes |
| `src/wip/root/root_047c.c` | root 047c | draft (not C; not built) | 0/87 | yes |
| `src/wip/root/text.c` | root 0265 | draft (--functional) | 8/10 | yes |
| `src/wip/s000/s000_0e76.c` | s000 0e76 | draft (--functional) | 8/11 | yes |
| `src/wip/s000/s000_0fb2.c` | s000 0fb2 | draft (--functional) | 2/2 FILE MATCH | yes |
| `src/wip/s001/s001_0e76.c` | s001 0e76 | draft (--functional) | 0/1 | yes |
| `src/wip/s002/s002_0e76.c` | s002 0e76 | draft (--functional) | 5/10 | no (uses `party` as a local) |
| `src/wip/s003/s003_0e76.c` | s003 0e76 | draft (--functional) | 4/8 | yes |
| `src/wip/s004/s004_0e76.c` | s004 0e76 | draft (--functional) | 4/11 | yes |
| `src/wip/s004/s004_1091.c` | s004 1091 | draft (--functional) | 9/13 | yes |
| `src/wip/s004/s004_1212.c` | s004 1212 | draft (--functional) | 2/2 FILE MATCH | yes |
| `src/wip/s004/s004_1223.c` | s004 1223 | draft (--functional) | 2/2 FILE MATCH | nothing to rename |
| `src/wip/s004/s004_1237.c` | s004 1237 | draft (--functional) | 4/4 FILE MATCH | yes |
| `src/wip/s004/s004_1262.c` | s004 1262 | draft (--functional) | 17/21 | yes |
| `src/wip/s004/s004_1336.c` | s004 1336 | draft (--functional) | 33/36 | yes |
| `src/wip/s005/s005_1432.c` | s005 1432 | draft (--functional) | 1/3 | yes |
| `src/wip/s005/s005_146d.c` | s005 146d | draft (--functional) | 0/15 | yes |
| `src/wip/s005/s005_1551.c` | s005 1551 | draft (--functional) | 0/5 | yes |
| `src/wip/s006/s006_15af.c` | s006 15af | draft (--functional) | 0/3 | yes |
| `src/wip/s006/s006_1637.c` | s006 1637 | draft (--functional) | 0/3 | yes |
| `src/wip/s006/s006_168e.c` | s006 168e | draft (--functional) | 0/2 | yes |
| `src/wip/s006/s006_16e1.c` | s006 16e1 | draft (--functional) | 0/2 | yes |
| `src/wip/s006/s006_17df.c` | s006 17df | draft (--functional) | 1/6 | yes |
| `src/wip/s006/s006_18fa.c` | s006 18fa | draft (--functional) | 0/5 | yes |
| `src/wip/s006/s006_1a57.c` | s006 1a57 | draft (--functional) | 0/3 | yes |
| `src/wip/s006/s006_1abf.c` | s006 1abf | draft (--functional) | 1/4 | yes |
| `src/wip/s007/s007_15af.c` | s007 15af | draft (--functional) | 5/34 | yes |
| `src/wip/s008/s008_1432.c` | s008 1432 | draft (--functional) | 7/16 | yes |
| `src/wip/s009/s009_1432.c` | s009 1432 | draft (--functional) | 12/32 | yes |
| `src/wip/s009/s009_182c.c` | s009 182c | draft (--functional) | 7/16 | yes |
| `src/wip/s010/s010_1432.c` | s010 1432 | draft (--functional) | 9/18 | yes |
| `src/wip/s011/s011_1432.c` | s011 1432 | draft (--functional) | 6/17 | yes |
| `src/wip/s012/s012_1432.c` | s012 1432 | draft (--functional) | 5/6 | yes |
| `src/wip/s013/s013_1432.c` | s013 1432 | draft (--functional) | 1/10 | yes |
| `src/wip/s014/s014_0e76.c` | s014 0e76 | draft (--functional) | 13/39 | yes |
| `src/wip/s014/s014_12f7.c` | s014 12f7 | draft (--functional) | 13/18 | no (`gamenames: off`: game.h changes its code) |
| `src/wip/s014/s014_1462.c` | s014 1462 | draft (--functional) | 0/5 | yes |
| `src/wip/s014/s014_14f7.c` | s014 14f7 | draft (--functional) | 8/27 | yes |
| `src/wip/s014/s014_18bf.c` | s014 18bf | draft (--functional) | 3/13 | yes |
| `src/wip/s014/s014_19cd.c` | s014 19cd | draft (--functional) | 10/34 | yes |
| `src/wip/s014/s014_1d47.c` | s014 1d47 | draft (--functional) | 0/2 | yes |
| `src/wip/s014/s014_1d69.c` | s014 1d69 | draft (--functional) | 1/3 | yes |
| `src/wip/s014/s014_1db7.c` | s014 1db7 | draft (--functional) | 8/16 | yes |
| `src/wip/s014/s014_1ec9.c` | s014 1ec9 | draft (--functional) | 0/4 | yes |
| `src/wip/s015/s015_1ef8.c` | s015 1ef8 | draft (--functional) | 4/7 | yes |
| `src/wip/s016/s016_1ef8.c` | s016 1ef8 | draft (--functional) | 56/59 | yes |
| `src/wip/s017/s017_1ef8.c` | s017 1ef8 | draft (--functional) | 0/1 | yes |
| `src/wip/s018/s018_1f43.c` | s018 1f43 | draft (--functional) | 2/6 | yes |
| `src/wip/s020/s020_1ef8.c` | s020 1ef8 | draft (--functional) | 1/3 | yes |
| `src/wip/s022/s022_1ef8.c` | s022 1ef8 | draft (--functional) | 3/5 | yes |
| `src/wip/s023/s023_1ef8.c` | s023 1ef8 | draft (--functional) | 3/16 | yes |
| `src/wip/s024/s024_1ef8.c` | s024 1ef8 | draft (--functional) | 0/4 | yes |
| `src/wip/s025/s025_1ef8.c` | s025 1ef8 | draft (--functional) | 6/7 | yes |
| `src/wip/s026/s026_1ef8.c` | s026 1ef8 | draft (--functional) | 2/2 FILE MATCH | yes |
| `src/wip/s026/s026_1f09.c` | s026 1f09 | draft (--functional) | 2/2 FILE MATCH | yes |
| `src/wip/s026/s026_1f1d.c` | s026 1f1d | draft (--functional) | 4/4 FILE MATCH | yes |
| `src/wip/s027/s027_1f48.c` | s027 1f48 | draft (--functional) | 17/21 | yes |
| `src/wip/s028/s028_1f48.c` | s028 1f48 | draft (--functional) | 33/36 | yes |
| `src/wip/s029/s029_0e76.c` | s029 0e76 | draft (--functional) | 136/143 | yes |
| `src/wip/s030/overland.c` | s030 1462 | draft (--functional) | 16/22 | yes |
| `src/wip/s031/s031_1462.c` | s031 1462 | draft (--functional) | 18/19 | yes |
| `src/wip/s031/s031_18e7.c` | s031 18e7 | draft (--functional) | 25/27 | yes |
| `src/wip/s032/barter.c` | s032 1462 | draft (--functional) | 8/11 | yes |
| `src/wip/s033/s033_1462.c` | s033 1462 | draft (--functional) | 9/16 | yes |
| `src/wip/s036/s036_1462.c` | s036 1462 | draft (--functional) | 19/23 | yes |
| `src/wip/s037/s037_1462.c` | s037 1462 | draft (--functional) | 14/20 | yes |
| `src/wip/s037/s037_18c8.c` | s037 18c8 | draft (--functional) | 10/15 | yes |
| `src/wip/s038/s038_1462.c` | s038 1462 | draft (--functional) | 5/7 | yes |
| `src/wip/s040/loot.c` | s040 1462 | draft (--functional) | 9/10 | yes |
| `src/wip/s043/s043_150b.c` | s043 150b | draft (--functional) | 24/28 | yes |

fmatch counts are per original source file (root files list every function of the original
segment file, e.g. `random.c` holds 1 of root 0410's 29). Every converted file gave exactly the
same per-function fmatch output (matches, first differing offsets, DATA status) before and after
the conversion; the before/after outputs are in `build/agent/n1/res/`.
