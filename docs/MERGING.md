# Taking from Darklands+ into another engine

Every switch, with what it does and why, is listed in `docs/SWITCHES.md` (made by the game from its own table; the keys never change).

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
| The members' ways of fighting (Skirmisher, Archer, Charger, Manual): their names, what each does, how they are kept | `port/mods/styles.c`; carried out in `port/mods/ways.c` |
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
parent, title, a line on what it does, and its **class** (`enum OptClass`, `port/mods/options.h`);
the bug fixes (`fix.<id>`) and the built-in rule changes (`mod.<id>`) are appended from their own
tables (`port/mods/fixes.c`, `port/mods/builtin.c`). The class decides what each preset sets
(`preset_value`): the original's value or Darklands+'s.

| Class | Meaning | Full Original | Purist | Darklands+ |
|---|---|---|---|---|
| fix | puts right a bug of the original | original | Darklands+ | Darklands+ |
| show | draws a screen anew or tells more; what happens is the game's own | original | original | Darklands+ |
| ease | saves clicks and waiting: what the player could do by hand | original | original | Darklands+ |
| plus | changes a rule a little, for less chore (one pack for the party ...) | original | original | Darklands+ |
| easy | makes the game easier: a built-in mod (F12 > Mods) | original | original | the player's own |
| experiment | not yet sure to work well | original | original | original |
| taste | a matter of taste (speed, typeface, pictures) | original | original | the player's own |
| tool | a tool (the editor) | the player's own | the player's own | the player's own |
| look | a look or a preference of the window, the sound, the saves | the player's own | the player's own | the player's own |

A switch in a group (`BUNDLES[]` in options.c: Easier battle controls, New screens ...) follows its
group's switch. **Full Original** also turns on the master switch `original`: the game as it was.
A switch the player changed by hand makes the preset "Custom"; the player's own choices are kept
and can be had back.

To add a feature: one line in `OPTIONS[]`, and the module reads its key (and `original`).

## 4. The switches, and where each lives

Made from `docs/SWITCHES.md` and the sources by `port/tools/switch_map.py` (`tools/check_docs.py --fix`):
the files where each switch's key is read.

| Switch | What | Class | Where |
|---|---|---|---|
| **New screens** | | | |
| `display.native_screens` | New screens | show | a group of switches: options.c (BUNDLES), settings.cpp |
| `display.shop_compare` | Wares compared with what is equipped | show | barterview.cpp, sheetview.cpp |
| `display.native_panel` | New party panel | show | partypanel.cpp, ui.cpp |
| `display.panel_picture` | Picture in the party panel | taste | partypanel.cpp, ui.cpp |
| `display.tooltip_delay` | Tooltip delay | taste | ui.cpp |
| `display.native_menu` | New menus | show | menuview.c |
| `display.native_title` | New start screen | show | titleview.c |
| `display.native_cards` | New card screens | show | cardview.c |
| `display.native_shops` | New shop screen | show | barterview.c |
| `display.native_sheet` | New character sheet | show | sheetview.c |
| `inventory.shared` | One shared pack for the party | plus | sheetview.c |
| `display.native_loot` | New loot screen | show | lootview.c |
| `assist.loot_worth` | Shop value in the loot screen | show | lootview.cpp |
| `display.native_alchemy` | New alchemy screen | show | alchemyview.c |
| `assist.where_sold` | Where to buy missing ingredients | show | alchemyview.cpp |
| `display.native_cache` | New store room screen | show | cacheview.c |
| `store.shared` | One store room shared by every inn | plus | sharedstore.c |
| `display.native_lodge` | New inn and camp screen | show | lodgeview.c |
| `display.native_pray` | New prayer screen | show | prayview.c |
| `display.native_create` | New character creation | show | createview.c |
| `assist.creation_guide` | Step-by-step guide | show | ui.cpp |
| `display.native_roster` | New party selection screen | show | rosterview.c |
| `display.native_info` | New Party Info screen | show | infoview.c |
| `display.native_march` | New marching order screen | show | marchview.c |
| `display.native_enemyinfo` | New Enemy Info screen | show | enemyview.c |
| `display.native_mapmsg` | New overland view messages | show | mapmsg.c |
| `display.native_saves` | New Save and Load screens | show | savesview.c |
| **Cards and towns** | | | |
| `assist.inn_earn` | At the inn, rested members earn money | ease | lodgeview.c |
| `assist.card_helpers` | Chances and hints on the cards | show | a group of switches: options.c (BUNDLES), optpreview.cpp |
| `assist.chances` | Chances of success | show | assist.c |
| `assist.best_leader` | The best member speaks for the party | ease | assist.c, sheetview.cpp |
| `assist.card_help` | Saints and items on cards | show | assist.c |
| `assist.shop_info` | Tooltips | show | sheetview.cpp, ui.cpp |
| `assist.fast_fades` | Cards without the fade-in | show | hdtext.c |
| `assist.card_keys` | Number keys for card options | show | assist.c |
| `assist.goto_work` | Who has work | show | citynav.cpp |
| `assist.teachers` | Teachers | show | teachersui.cpp |
| `assist.city_goto` | Go to (G) | ease | margins.cpp, ui.cpp |
| `assist.docks_map` | Map at the docks | ease | cardview.cpp |
| `journal.enabled` | Note each town's shops and wares | look | journal.cpp, ui.cpp |
| `journal.chronicle` | Chronicle | look | journal.cpp |
| **The party and its items** | | | |
| `rules.ingredients_one_quality` | Ingredients of one quality | plus | loot.c |
| `assist.return_thrown` | Thrown weapons picked up after a battle | ease | loot.c |
| `assist.endless` | Endless ammunition and thrown weapons | plus | a group of switches: options.c (BUNDLES) |
| `assist.infinite_ammo` | Endless ammunition | plus | ammo.c |
| `assist.infinite_thrown` | Endless thrown weapons | plus | ammo.c |
| `rules.aging` | Aging during play | plus | aging.c |
| `assist.loss_alerts` | Warn about permanent losses | show | losses.c |
| `assist.more_jobs` | More day jobs | ease | lodgeview.c |
| **Battles** | | | |
| `battle.controls` | Easier battle controls | ease | a group of switches: options.c (BUNDLES), optpreview.cpp |
| `battle.helpers` | Battle info on screen | show | a group of switches: options.c (BUNDLES), optpreview.cpp |
| `battle.pauses` | Pauses in battle | ease | a group of switches: options.c (BUNDLES) |
| `battle.scrolling` | Smooth, steady scrolling | show | a group of switches: options.c (BUNDLES) |
| `battle.pause_down` | Pause when a member falls | ease | autopause.c |
| `battle.pause_hurt` | Pause when badly hurt | ease | autopause.c |
| `battle.pause_foes` | Pause when enemies come out | ease | autopause.c |
| `battle.improved` | Improved combat | ease | battlerules.c, ui_windows.cpp |
| `battle.click_attack` | Click an enemy to attack it | ease | orders.c |
| `battle.volley_toggle` | Way of fighting on each member's box | show | battleoverlay.cpp, partypanel.cpp, ui_windows.cpp |
| `battle.panel_select` | Click a member's box to give them orders | ease | partypanel.cpp, ui_windows.cpp |
| `battle.quick_orders` | One click gives an order | ease | orders.c, battleoverlay.cpp, margins_battle.cpp, tips.cpp, ui_windows.cpp |
| `battle.next_member` | Then select the next member without orders | ease | orders.c |
| `battle.order_bar` | Order buttons over the field | show | margins_battle.cpp |
| `battle.order_icons` | Order buttons as pictures | show | margins_battle.cpp |
| `battle.los_line` | Show the line of a shot | show | orders.c |
| `battle.click_hint` | Say what a click will do | show | orders.c |
| `battle.drag_pan` | Drag to move the view | show | platform_sdl.c |
| `battle.select_stays` | Selecting a member doesn't move the view | show | battlescroll.c |
| `battle.smooth_scroll` | Smooth scrolling | show | platform_sdl.c |
| `battle.scroll_paced` | Steady scrolling | show | battlescroll.c |
| `battle.scroll_rest` | Edge scrolling without Shift | show | battlescroll.c |
| `battle.info` | Info under the pointer | show | battleoverlay.cpp, ui.cpp |
| `battle.foe_arrow` | Arrow to enemies off screen | ease | battleoverlay.cpp |
| `battle.skill_gains` | Show skill gains | show | battleoverlay.cpp |
| `battle.reload_bar` | Reload bar | show | battleoverlay.cpp |
| `battle.log` | Battle log | show | ui.cpp |
| `assist.level_map` | Level map (Tab) | show | levelmap.cpp, optpreview.cpp |
| `battle.end_words` | Shorter wait at the end of a battle | show | battleend.c |
| `fix.pathfinding` | Fighters find their way around obstacles | fix | pathfix.c |
| `battle.speed` | Battle speed | taste | pathfix.c |
| `battle.fast_key` | Fast forward (Z) | show | margins_battle.cpp, tips.cpp, ui.cpp, ui_windows.cpp |
| **Easier play** | | | |
| `battle.info_numbers` | Enemy Endurance as numbers | easy | ui.cpp |
| `assist.level_map_all` | The whole level on the level map | easy | levelmap.cpp |
| `battle.foes_left` | Enemies still waiting | easy | foesleft.c |
| `map.offers_unvisited` | What unvisited places offer | easy | journal.cpp, libraries.cpp, worldmap.cpp |
| `travel.encounters` | Encounters on the road | easy | travel.c |
| `travel.street_encounters` | Night encounters in the streets | easy | travel.c, margins.cpp |
| `mod.swift_travel` | Faster overland travel | easy | builtin.c |
| `mod.cheap_inn` | Half-price inns | easy | builtin.c |
| `mod.quick_learning` | Quick learning | easy | builtin.c |
| `mod.heavy_blows` | Double damage against enemies | easy | builtin.c |
| `mod.strong_party` | Strong Quickstart party | easy | builtin.c |
| `mod.rich_start` | Rich start | easy | builtin.c |
| **Travel** | | | |
| `map.key_m` | World map (M) | show | settings.cpp, ui.cpp |
| `travel.auto` | Auto-travel | ease | travel.c, worldmap_draw.cpp |
| `travel.fast` | Auto-travel speed | taste | travel.c |
| `map.overland_marks` | Names and quest markers in the overland view | show | a group of switches: options.c (BUNDLES), settings.cpp |
| `map.names` | Place names in the overland view | show | optpreview.cpp, worldmap_draw.cpp |
| `map.quest_marks` | Quest markers in the overland view | show | optpreview.cpp, worldmap_draw.cpp |
| **Elsewhere** | | | |
| `display.text_font` | Typeface | look | settings_look.cpp, ui.cpp |
| `display.overlay_font_follow` | The same typeface for the margins | look | settings_look.cpp, ui.cpp |
| `display.spell_out` | Spell out abbreviations | look | settings_look.cpp, ui.cpp |
| `display.coins` | Money as coins | look | money.cpp, settings_look.cpp |
| `display.text_edge` | Letter edges | look | settings_look.cpp, ui.cpp |
| `display.text_contrast` | Darken faint text | look | settings_look.cpp, ui.cpp |
| `display.text_size` | Text size | look | margins.cpp, settings_setup.cpp |
| `display.skill_icons` | Icons beside the skills | look | settings_look.cpp, ui.cpp |
| `display.skill_icon_look` | Skill icons' look | look | settings_look.cpp, skillicons.cpp |
| `display.potion_grade` | A potion's grade | look | alchemyicons.cpp, nsdraw.cpp, settings_look.cpp |
| `display.pstone_look` | The Philosopher's Stone | look | pstone.cpp |
| `display.clock` | Clock | look | lodgeview.c, hourglass.cpp, settings_look.cpp |
| `display.ornate` | Ornaments | look | ornament.cpp, settings_look.cpp |
| `display.frame_style` | Frame style | look | ornament.cpp, settings_look.cpp |
| `display.button_style` | Button style | look | ornament.cpp, settings_look.cpp |
| `display.art_names` | Replacement art: show file names | look | hdart.c, settings.cpp |
| `display.smooth` | Soft scaling | look | settings_setup.cpp, ui.cpp |
| `display.scaler` | Rounded edges | look | settings_setup.cpp, ui.cpp |
| `display.system_cursor` | System pointer | look | settings_setup.cpp, ui.cpp |
| `display.live_pointer` | Smooth pointer | look | overlay_text.cpp |
| `display.ui_scale` | Size of the menus | look | settings_setup.cpp, ui.cpp |
| `display.pause_unfocused` | Pause in the background | look | settings_setup.cpp, platform_sdl.c |
| `display.opening` | Intro at start-up | look | opening.c |
| `display.draw_cache` | Drawing cache | look | drawcache.cpp |
| `encounters.new` | Encounters from mods | look | encounters.c |
| `encounters.seal` | A seal on encounters from mods | look | overlay.cpp, settings.cpp |
| `encounters.share` | How often encounters from mods come up | look | encounters.c, settings.cpp |
| `input.prompts` | Button prompts | look | settings_setup.cpp, platform_sdl.c |
| `saves.ironman` | Ironman | look | saves.c, settings.cpp, ui_windows.cpp |
| `saves.backup` | Back up saves | look | saves.c |
| `saves.keep_auto` | Autosaves kept | look | saves.c, saves.cpp |
| `saves.keep_quick` | Quick saves kept | look | saves.c, saves.cpp |
| `sound.music_volume` | Music volume | look | settings_setup.cpp, sound.c |
| `sound.effects_volume` | Effects volume | look | sfx.c |
| `sound.music_in_menus` | Music in the menus | look | bios.c |
| `sound.mt32_lcd` | MT-32 display | look | overlay.cpp, settings_setup.cpp, mt32.c |
| `quests.sort` | Quests sorted by | look | margins.cpp, quests.cpp |
| `display.crisp_text` | New text | taste | hdtext.c |
| `display.hd_art` | Replacement art | taste | hdart.c |
| `sound.effects` | Battle sounds from a sound pack | look | sfx.c |
| `display.margins` | Side margins | show | settings_setup.cpp, ui.cpp |
| `display.full_names` | Names in full | show | gamedata.cpp, settings_look.cpp, ui.cpp |
| `display.armour_worn` | Where armor is worn | show | gamedata.cpp, settings_look.cpp, ui.cpp |
| `saves.quick` | Quick saves and autosaves | ease | saves.c |
| `saves.autosave_arrive` | Autosave on arriving | ease | saves.c, saves.cpp |
| `saves.autosave_battle` | Autosave before a battle | ease | saves.c, saves.cpp |
| `saves.restart_on_end` | Back to the start screen when the journey ends | ease | saves.c, saves.cpp |
| **Reconstructed Content** | | | |
| `rules.fehde` | "Fehde!" ambush | experiment | cutcards.c |
| `rules.hospice` | Town hospice | experiment | cutcards.c |
| `rules.war` | Cities at war | experiment | war.c, settings.cpp |
| `rules.politics` | City politics | experiment | politics.c, quests.cpp, settings.cpp |
| `battle.black_cloud` | Black Cloud in battle | experiment | battlerules.c, builtin.c, settings.cpp |
| `assist.hall_of_fame` | Honor Roll of Glory (hall of fame) | experiment | builtin.c, halloffame.cpp, journal.cpp |
| **Fixes for bugs of the original** | | | |
| `fix.blessing` | Blessings no longer cost attribute points for good | fix | fixes.c, nsdraw.cpp |
| `fix.fireward` | No invulnerability from fire protection | fix | fixes.c, saintinfo.c, nsdraw.cpp, sheetview.cpp |
| `fix.nullitem` | No "(null)" or empty items | fix | fixes.c |
| `fix.castle` | Castle battles end when the knight dies | fix | fixes.c, worldmap_draw.cpp |
| `fix.bounty` | Robber-knight bounties from merchants pay out properly | fix | fixes.c |
| `fix.alchemy` | Brewing uses up all the ingredients | fix | encounters.c, fixes.c, screenpics.cpp, sheetview.cpp |
| `fix.fullpack` | No more items lost to a member with no room | fix | fixes.c |
| `fix.innsave` | Saving and loading stay possible after the inn | fix | fixes.c |
| `fix.colors` | Members keep their colors (the "neon hair" bug) | fix | fixes.c |
| `fix.copyprot` | Skip the manual check | fix | fixes.c |
| `fix.potions` | Weapon potions wear off and don't ruin weapons | fix | fixes.c, lodgeview.h, alchemyview.cpp, nsdraw.cpp |
| `fix.mapfoot` | No jump at the foot of the travel map | fix | fixes.c, travel.c |
| `fix.offmap` | No quest destinations off the edge of the land | fix | fixes.c |
| `fix.bestweapon` | Weapon practice trains the leader's best weapon skill | fix | fixes.c |
| `fix.itemcount` | Robbers take only what is there (no crash afterwards) | fix | fixes.c |
| `fix.store room` | Nothing lost in store rooms | fix | fixes.c |
| `fix.testcard` | No crash on the developers' test card | fix | fixes.c |
| `fix.armorwear` | Only the armor worn wears out | fix | fixes.c |
| `fix.teachers` | Training at the inn finds the town's teachers | fix | fixes.c, lodgeview.c |
| `fix.cardopts` | Card options no longer grayed out by mistake | fix | fixes.c, war.c, cardhints.cpp, citynav.cpp, margins.cpp |
| `fix.patronsaint` | Saints taught by a cathedral, or by St. Cosmas or St. Damian, are learned | fix | fixes.c |
| `fix.debtnote` | Debt notes are where the merchant says | fix | fixes.c, quests.cpp |
| `fix.alchrisk` | The risk levels of alchemy formulae work the right way around | fix | fixes.c |
| `fix.bishop` | Quests from a cathedral are the bishop's, not the merchants' | fix | fixes.c |
| `fix.altar` | No quests to a purged pagan altar | fix | fixes.c |
| `fix.tomb` | The item taken from a tomb goes to the winner of the fight | fix | fixes.c, worldmap_draw.cpp |
| `fix.lordreward` | The lord pays for his quests | fix | fixes.c, citynav.cpp |
| `fix.armorday` | Armor and fire protection from potions and saints last a day; Hardarmor works as the manual says | fix | fixes.c, saintinfo.c, sainttexts.c, nsdraw.cpp, sheetview.cpp |
| `fix.greatpower` | Great Power raises the weapon's quality | fix | fixes.c, nsdraw.cpp, saints.cpp |
| `fix.standings` | Saved games keep the party's standing with the Fugger, the Medici and the Hanse | fix | fixes.c |
| `fix.battlesaints` | A saint's help called in a battle wears off | fix | fixes.c |
| `fix.healboost` | Healing no longer takes a boost away | fix | fixes.c |
| `fix.teachcap` | Teachers teach only up to their own ability | fix | fixes.c, teachers.c, teachers.h, citynav.cpp, lodgeview.cpp, margins.cpp, teachersui.cpp |
| `fix.oilweapons` | Each weapon oil works with the weapons the manual names | fix | fixes.c, nsdraw.cpp |
| `fix.pauls` | The two St. Pauls do what their texts say | fix | fixes.c, saintinfo.c, sheetview.cpp |
| `fix.saintrep` | A saint's reputation bonus goes to the right town | fix | fixes.c, saintinfo.c, sheetview.cpp |
| `fix.sainttexts` | Saints do what their texts say | fix | fixes.c, saintinfo.c, sainttexts.c, sainttexts.h, sheetview.cpp |
| `fix.nightbowyer` | The bowyer sells bows at night too | fix | fixes.c |
| `fix.confession` | Confession always takes at least an hour | fix | fixes.c |
| `fix.appointment` | An audience promised for tomorrow is kept | fix | fixes.c |
| `fix.pileup` | Many blows at once on one fighter all count | fix | fixes.c |
| `fix.castlefreeze` | No freeze when placing the party in a castle | fix | fixes.c |
| `fix.university` | A failed search for formulae at a university shuts that search, not the saints' | fix | fixes.c, cardhints.cpp, citynav.cpp |
| `fix.partyitems` | An item given to the whole party is given once | fix | fixes.c |
| `fix.itemflags` | A mount's quality helps when fleeing on horseback | fix | fixes.c |
| `fix.membervalue` | A church tithe blesses the right member | fix | fixes.c |
| `fix.cardslips` | Small mistakes in single cards | fix | fixes.c |
| `fix.nightwall` | Climbing a city wall at night uses the right chances | fix | fixes.c |
| `fix.campguard` | A guard skilled with impact weapons keeps the camp safer | fix | fixes.c, lodgeview.c |
| `fix.enemygroups` | No group of enemies goes missing from a battle | fix | fixes.c |
| `fix.cityalert` | A town on alert remembers the trouble | fix | fixes.c, quests.cpp |
| `fix.dayjobs` | The friars' day jobs are offered | fix | fixes.c |
| `fix.cloudline` | Shots through smoke clouds are tested correctly | fix | fixes.c |
| `fix.attrsum` | Cards that test two attributes add both | fix | fixes.c |
| `fix.lootcount` | No loot lost past 255 of a kind | fix | fixes.c |
| `fix.corridors` | Castle corridor decorations in the right place | fix | fixes.c |
| `fix.battledead` | A hireling killed in battle doesn't later send away his replacement | fix | fixes.c |
| `fix.createback` | Esc in character creation doesn't reopen a childhood already chosen | fix | fixes.c |
| `fix.ruinkeep` | The burnt-out keep lets the party go after searching it | fix | fixes.c |
| `fix.rkonce` | A giver asks for a robber knight once | fix | fixes.c |
| `fix.bestteacher` | Of two teachers of one skill, the better one teaches | fix | fixes.c, teachers.c, lodgeview.cpp |
| `fix.minerelic` | A relic found in a mine is as good as the relic itself | fix | fixes.c |
| `fix.ownplace` | A village beside another place can be entered | fix | fixes.c |
| `fix.hitsound` | Ordinary hits make a sound | fix | fixes.c, sfx.c |

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
