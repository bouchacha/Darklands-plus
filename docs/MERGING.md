# Taking from Darklands+ into another engine

This is for the authors of other Darklands projects, first of all **Darklands Restoration**
(Arkana Mechanika, C# and SDL, its own core). Darklands+ runs the original game program,
recompiled function by function, and changes it from the outside; a restoration with its own
core cannot take that layer as it is. What it can take is in three parts, from the most portable
to the least:

1. **What the original really does**: rules, formulas, bugs and their fixes, read from the game's
   code and written down beside each feature with the function's address. This is most of the
   value and needs no code at all.
2. **Logic that asks only the game's state**: plain C with no window, no ImGui, no SDL. It reads
   the game through a handful of accessors; give those accessors your own model and the files
   compile as they are, or translate them line by line.
3. **The look of the screens**: ImGui code tied to this port; take the designs (the screenshots
   in the release notes, the layouts described at the top of each `port/ui/*.cpp`), not the code.

Every feature is a switch, and each switch has a class that decides which presets turn it on
(below). Keeping the same keys and classes in both projects means a player's choice ("Purist",
"Darklands+ but without aging") means the same thing in both.

## 1. Knowledge: where it is written

Each file's header says what the original does, with the function's address
(`s014 1db7:0e22` = overlay section 14, segment 1db7, offset 0e22 in the 483.07 program;
`src/c`, `src/wip` and `build/asm` hold the decompiled and disassembled code by the same names).

| Subject | Where |
|---|---|
| Bugs of 483.07 and their fixes (about 50: blessings, fire protection, weapon oils, armour potions, Great Power, castle battles, bounties, the night wall's chances, the Hussites' friendship, slips in single cards, alchemy, teachers at the inn, debt notes, the bishop's errands, purged altars, tomb fights, saints called in battle, the companies' standing in saves ...) | `port/mods/fixes.c` (header, then a section per fix) |
| Darklands Restoration's register of original bugs, entry by entry: fixed here or not, and the switch | `docs/register_crosscheck.md` |
| Every potion: the code, the manual, what the fixes change | `docs/POTIONS.md` |
| Every saint: the code, the texts, what the fixes change (`port/mods/saintinfo.c` tells it in the game) | `docs/SAINTS.md` |
| The 43 saints made to do what their texts say (travel speed, fire, weapon and armour quality ...) and why | `docs/SAINTS.md`, `port/mods/sainttexts.c` |
| Why each greyed card option is greyed (280+ reasons from the cards' scripts) | `port/ui/cardhints_data.h`, `port/ui/cardhints.cpp` |
| What each potion really does (which weapons, how long, the original's overflow) | `ns_potion_effect` in `port/ui/nsdraw.cpp` |
| Teachers: who brings one, the skills, ability, the daily chance, fees, conditions | `port/mods/teachers.h` |
| Task givers: when the Fugger, Medici, Hanse, canon, council, lord and stalls give tasks, the locks after a refusal | `port/mods/offices.h` (and the research in its header) |
| Aging: the creation table and how it ages characters | `port/mods/aging.c` |
| Permanent losses (attributes, skills, armour wear) | `port/mods/losses.c` |
| Battle AI: choosing the next foe; line of fire; shooters | `port/mods/battlerules.c` |
| Pathfinding: the original's wall-following trackers and their limits | `port/mods/pathfix.c` |
| The order a click gives in battle; weapon types and the skills they use | `port/mods/orders.c` |
| The end of a battle (how long the field is held) | `port/mods/battleend.c` |
| Saints taught by each library, formulae sold by each alchemist (fixed per game: the seed formulas) | `port/mods/gamemodel.h`, `port/ui/libraries.cpp` |
| The game's state by meaning (party, items, what is in use, what is known, events) | `port/mods/gamemodel.h`, `port/runtime/game.h` (`g_event`, the `G_EV_` fields) |
| Names of the game's variables and functions | `include/darklands/*.h`, generated into `port/runtime/gamevars.h` |
| Card scripts; the data files' formats (Python readers) | `docs/CARDS.md`, `formats/dlformats/` |

## 2. Logic that asks only the game's state

These files include no interface. They read the game through:

* `g_rb / g_rw` (a byte or word of the game's data segment), `rb / rw` (any segment),
* `g_event(i)` and `g_ev(e, field)` (the 300 timed events),
* `g_location(i)` (a place's record), the `G_MEMBER(m)` record of a party member,
* and, for a few, `ns_call` (a game routine: prices, line of fire, can it fire);
* they tell the player through `ui_alert(title, body, foot)` (a window to click through), which the host provides.

| File | What it gives | Needs |
|---|---|---|
| `port/mods/options.c/.h` | the table of every switch and the presets (see 3) | `config_get_int / config_set_int` |
| `port/mods/teachers.c/.h` | the teachers a town can offer, the card options that bring them, those hired now | events, the town's record, party skills |
| `port/mods/offices.c/.h` | whether each task giver has work now, or until when none | events, the place's record, fame, party |
| `port/mods/aging.c` | the years' toll, month by month | the age, the birth month, attribute maxima |
| `port/mods/losses.c` | what was lost for good, to tell the player | maxima and skills, events 0x48 / 0x49 |
| `port/mods/gamemodel.c/.h` | members, items, what is known, library saints, the hours' names | the party and item records |
| `port/mods/sainttexts.c/.h` | each of 43 saints' help as its text says; the party's travel speed from it | `ns_call` (the saints' routines), events, a hook on the minutes of a step |

For an engine with its own model, write those accessors over it (or replace each read by the
model's field: the offsets are named in the comments) and keep the logic. Where a file calls a
game routine (`ns_call`), the routine's address says what to call in yours.

## 3. Switches, classes and presets

Every feature is one line of `OPTIONS[]` in `port/mods/options.c`: its config key, section,
parent, title, a line on what it does, and its **class**; the bug fixes (`fix.<id>`) and the
built-in rule changes (`mod.<id>`) are appended from their own tables. The class decides the
presets:

| Class | Meaning | On from |
|---|---|---|
| fix | a bug of the original fixed | Purist |
| interface | shows more, changes no rule | Purist + interface |
| convenience | saves the player work; a small change to how the game plays (the best member speaks, a click attacks) | Semi-Purist |
| new rule | changes how the game plays (aging, one pack for the party) | Darklands+ |
| easier | makes the game easier (cheap inns, heavy blows), or tells what the party cannot know (what unvisited places offer, the foes not yet met) | Easy only |
| experimental | not finished, or content MicroProse never switched on (restored cut content: its own page) | never by a preset |
| taste | a matter of taste (speed, pictures): the original in the purist presets, else the player's own | - |
| tool | the editor and the like: never set by a preset | - |

**Buggy Purist** is everything off and the master switch `original` on: the game as it was.
**Easy** takes each switch's own "easy" value. A switch the player changed by hand makes the
preset "Custom"; the player's own choices are kept and can be had back.

To add a feature: one line in `OPTIONS[]`, and the module reads its key (and `original`).

## 4. The switches, and where each lives

| Switch | What | Class | Where |
|---|---|---|---|
| **New screens** | | | |
| `display.native_panel` | New party panel | interface | partypanel.cpp, ui.cpp |
| `display.panel_picture` | Picture in the party panel | taste | partypanel.cpp, ui.cpp |
| `display.native_menu` | New menus | interface | menuview.c |
| `display.native_title` | New start screen | interface | titleview.c |
| `display.native_cards` | New card screens | interface | cardview.c |
| `display.native_shops` | New shop screen | interface | barterview.c |
| `display.native_sheet` | New character sheet | interface | sheetview.c |
| `inventory.shared` | One shared pack for the party | new rule | sheetview.c |
| `display.native_loot` | New loot screen | interface | lootview.c |
| `assist.loot_worth` | Shop value in the loot screen | interface | lootview.cpp |
| `display.native_alchemy` | New alchemy screen | interface | alchemyview.c |
| `assist.where_sold` | Where to buy missing ingredients | interface | alchemyview.cpp |
| `display.native_cache` | New store room screen | interface | cacheview.c |
| `store.shared` | One store room shared by every inn | new rule | sharedstore.c |
| `display.native_lodge` | New inn and camp screen | interface | lodgeview.c |
| `display.native_pray` | New prayer screen | interface | prayview.c |
| `display.native_create` | New character creation | interface | createview.c |
| `assist.creation_guide` | Step-by-step guide | interface | ui.cpp |
| `display.native_roster` | New party selection screen | interface | rosterview.c |
| `display.native_info` | New Party Info screen | interface | infoview.c |
| `display.native_march` | New marching order screen | interface | marchview.c |
| `display.native_enemyinfo` | New Enemy Info screen | interface | enemyview.c |
| `display.native_mapmsg` | New travel map messages | interface | mapmsg.c |
| `display.native_saves` | New Save and Load screens | interface | savesview.c |
| **Cards and towns** | | | |
| `assist.chances` | Chances of success | interface | assist.c |
| `assist.best_leader` | The best member speaks for the party | convenience | assist.c, sheetview.cpp |
| `assist.card_help` | Saints and items on cards | interface | assist.c |
| `assist.shop_info` | Tooltips | interface | sheetview.cpp, ui.cpp |
| `assist.fast_fades` | Cards without the fade-in | interface | hdtext.c |
| `assist.card_keys` | Number keys for card options | interface | assist.c |
| `assist.goto_work` | Who has work | interface | citynav.cpp |
| `assist.teachers` | Teachers | interface | teachersui.cpp |
| `assist.city_goto` | Go to (G) | convenience | margins.cpp, travel.cpp, ui.cpp |
| `journal.enabled` | Town journal | interface | journal.cpp, ui.cpp |
| `journal.chronicle` | Chronicle | interface | journal.cpp |
| **The party and its items** | | | |
| `assist.return_thrown` | Thrown weapons come back | convenience | loot.c |
| `assist.infinite_ammo` | Endless ammunition | new rule | ammo.c |
| `assist.infinite_thrown` | Endless thrown weapons | new rule | ammo.c |
| `assist.loot_all` | Loot and shop keys | interface | loot.c |
| `party.new_member_looks` | A sprite of their own for each new member | interface | memberlooks.c, rosterview.c |
| `party.keep_looks` | Characters keep their sprite and colours | interface | memberlooks.c |
| `party.more_figures` | Enemy sprites for your own characters | new rule | looks.c |
| `rules.aging` | Aging during play | new rule | aging.c |
| `assist.loss_alerts` | Warn about permanent losses | interface | losses.c |
| `party.figure_picker` | Sprite picker | interface | looks.c |
| **Battles** | | | |
| `battle.pause_down` | Pause when someone falls | convenience | autopause.c |
| `battle.pause_hurt` | Pause when badly hurt | convenience | autopause.c |
| `battle.pause_foes` | Pause when enemies come out | convenience | autopause.c |
| `battle.click_attack` | Click an enemy to attack it | convenience | orders.c |
| `battle.click_best` | Attack with the better weapon | convenience | orders.c |
| `battle.quick_orders` | One click gives an order | convenience | orders.c |
| `battle.next_member` | Then select the next member without orders | convenience | orders.c |
| `battle.order_bar` | Order buttons over the field | interface | margins.cpp |
| `battle.order_icons` | Pictures on the order buttons | interface | margins.cpp, ordericons.cpp |
| `battle.click_hint` | Say what a click will do | interface | orders.c, battleoverlay.cpp |
| `battle.scroll_paced` | Steady scrolling | interface | battlescroll.c |
| `battle.scroll_rest` | Edge scrolling without Shift | interface | battlescroll.c |
| `battle.info` | Info under the pointer | interface | ui.cpp |
| `battle.info_numbers` | Enemy Endurance as numbers | easier | ui.cpp |
| `battle.bars` | Health bars over the fighters | interface | ui.cpp |
| `battle.bars_hurt` | Bars only over the wounded | interface | battleoverlay.cpp |
| `battle.skill_gains` | Show skill gains | interface | battleoverlay.cpp |
| `battle.reload_bar` | Reload bar | interface | battleoverlay.cpp |
| `battle.log` | Battle log | interface | ui.cpp |
| `assist.level_map` | Level map (Tab) | interface | levelmap.cpp |
| `assist.level_map_all` | Show the whole level | easier | levelmap.cpp |
| `assist.level_map_walk` | Click the map to walk there | convenience | levelmap.cpp, margins.cpp |
| `battle.foes_left` | Enemies still waiting | new rule | foesleft.c |
| `battle.end_words` | Shorter end of battle | interface | battleend.c |
| `battle.retarget_near` | Next enemy within reach | convenience | battlerules.c |
| `battle.shoot_past_allies` | Shoot past friends | easier | battlerules.c |
| `battle.reposition_shooters` | Shooters move for a clear shot | experimental | battlerules.c, partypanel.cpp |
| `fix.pathfinding` | Better pathfinding | fix | pathfix.c |
| `battle.speed` | Battle speed | taste | pathfix.c |
| `battle.fast_key` | Fast forward (Z) | interface | margins.cpp, ui.cpp |
| **Travel** | | | |
| `travel.auto` | Auto-travel | convenience | travel.c |
| `travel.fast` | Auto-travel speed | taste | travel.c |
| `map.names` | Place names on the map | interface | worldmap.cpp |
| `map.quest_marks` | Quest markers on the map | interface | worldmap.cpp |
| `map.offers_unvisited` | What unvisited places offer | easier | libraries.cpp (place_offers_known), journal.cpp |
| `map.key_m` | World map (M) | interface | travel.cpp, ui.cpp |
| `travel.encounters` | Encounters on the road | easier | travel.c |
| `travel.street_encounters` | Night encounters in the streets | easier | margins.cpp, travel.c |
| **Controls** | | | |
| `assist.wheel_lists` | Mouse wheel in lists | interface | platform_sdl.c |
| `assist.menu_click` | Right-click for the menus | interface | platform_sdl.c, ui.cpp |
| `input.gamepad` | Game controller | interface | platform_sdl.c, settings.cpp, ui.cpp |
| **Editing tools** | | | |
| `editor.show` | Editing tools | tool | settings.cpp, ui.cpp |
| **Elsewhere** | | | |
| `display.crisp_text` | Crisp text | taste | hdtext.c |
| `display.hd_art` | Pictures | taste | hdart.c |
| `sound.effects` | Sound effects | taste | sfx.c |
| `display.margins` | Margins | interface | settings.cpp, ui.cpp |
| `display.full_names` | Names in full | interface | gamedata.cpp, settings.cpp, ui.cpp |
| `display.armour_worn` | Where armour is worn | interface | gamedata.cpp, settings.cpp, ui.cpp |
| `saves.quick` | Quick saves | convenience | saves.c |
| `saves.autosave_arrive` | Autosave on arriving | convenience | saves.c, saves.cpp |
| `saves.autosave_battle` | Autosave before a battle | convenience | saves.c, saves.cpp |
| `saves.restart_on_end` | Start again at the end | convenience | saves.c, saves.cpp |
| **Restored cut content** (their own page, F12 > Settings > Restored) | | | |
| `rules.fehde` | "Fehde!" ambush | experimental | cutcards.c |
| `rules.hospice` | Town hospice | experimental | cutcards.c, encounters.c, citynav.cpp |
| `rules.war` | Cities at war | experimental | war.c |
| `rules.politics` | City politics | experimental | politics.c, quests.cpp |
| `battle.black_cloud` | Black Cloud in battle | experimental | battlerules.c |
| `assist.hall_of_fame` | Honor Roll of Glory (hall of fame) | experimental | halloffame.cpp, journal.cpp |

The fixes (`fix.<id>`) are listed with their explanations in `port/mods/fixes.c`; the built-in
rule changes (`mod.<id>`) in `port/mods/builtin.c`.

## 5. What does not carry over

* **Hooks** (`hook_install(section, segment, offset, fn)`) wrap the original's functions: in an
  engine of its own, the same change goes into that engine's function for the same job (the
  address names the original's, and `src/c` / `build/asm` show what it did).
* **Native screens** (`port/mods/<screen>.c` with `port/ui/<screen>.cpp`) drive the original's
  screen routines (`docs/NATIVE_SCREENS.md`); their models and layouts are worth reading, their
  loops are not portable.
* The runtime (`port/runtime/`), the recompiler and the game code it makes are this port's own.

## 6. Keeping the two in step

* Keep the switch keys and classes the same where a feature exists in both.
* A bug found in either is a line in `port/mods/fixes.c`'s header with the address and the
  cause: the other side can fix it in its own way from that.
* The research notes for a feature are at the top of its file; a correction there helps both.
