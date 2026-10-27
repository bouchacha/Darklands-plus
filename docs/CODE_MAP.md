# Where things are in the code

Each source file of `port/`, with the first sentence of its header comment and its length. Made by
`port/tools/code_map.py`; run it again after adding or splitting a file. The switches are listed in
`docs/SWITCHES.md`; the original game's modules in `docs/MODULE_MAP.md`.

## runtime

The runtime: the CPU and DOS the game runs on, the window, sound, saves, the configuration.

| File | Lines | What it is |
|---|---:|---|
| `runtime/bios.c` | 801 | interrupts, BIOS services and the PC hardware the game talks to (VGA DAC, PIT, keyboard controller, PIC), plus the safe-point dl_poll() that delivers timer and keyboard interrupts to the game's own handlers and refreshes the screen. |
| `runtime/config.c` | 96 | settings kept in darklands_port.ini next to the game (key = value lines) |
| `runtime/cpu.c` | 202 | flags and arithmetic helpers for recompiled code |
| `runtime/dispatch.c` | 192 | calls and jumps to computed addresses |
| `runtime/dos.c` | 837 | high-level emulation of the DOS services the game uses (int 21h) |
| `runtime/drivers.c` | 98 | the run-time loaded driver images (MGRAPHIC.EXE, MISC.EXE, *SOUND.DLC). |
| `runtime/ems.c` | 211 | LIM EMS 4.0 expanded memory (int 67h), which Darklands requires. |
| `runtime/gamecall.c` | 192 | calling the game's own functions from native code (the new interface uses the game's inventory routines so equipment, weights and stacks stay consistent). |
| `runtime/gamefind.c` | 193 | where the game is when darklands.exe is not in the Darklands folder: a folder chosen before (darklands_plus/game_folder.txt next to the program), the GOG and Steam installations the registry knows, the usual places, and on Windows a folder dialog. |
| `runtime/gameload.c` | 92 | the program without the generated code (DL_RTSPLIT, see split.h) loads the game module the player's machine made from DARKLAND.EXE: darklands_game.dll (Windows) or libdarklands_game.so, in the game folder, or where DL_GAME_MODULE says. |
| `runtime/gensplit.c` | 76 | the game module's side of split.h (compiled with DL_GENSPLIT, together with the generated code, cpu.c and overrides.c): the runtime's variables are pointers set here, its functions are reached through the stubs below, and dl_game_init hands the generated tables back. |
| `runtime/hooks.c` | 110 | see hooks.h |
| `runtime/interp.c` | 423 | fallback x86 real-mode interpreter. |
| `runtime/main.c` | 355 | start-up of the native Darklands: load the original DARKLAND.EXE image into the emulated address space (the recompiled code reads its data, code-segment tables and overlay sections from there), set up DOS structures, and run the program. |
| `runtime/mpu.c` | 166 | MPU-401 MIDI interface (ports 330h/331h). |
| `runtime/mt32.c` | 380 | a Roland MT-32 / CM-32L / LAPC-I inside Darklands+ (Munt's libmt32emu, LGPL 2.1+, third_party/mt32emu). |
| `runtime/native.c` | 35 | native functions the game calls through far pointers (native.h) |
| `runtime/overrides.c` | 31 | hand-written replacements for the few original routines whose tricks the static recompiler cannot express (listed in port/recomp/overrides.txt). |
| `runtime/ovl.c` | 208 | RTLink/Plus overlay sections: loaded into memory on demand, as RTLink does. |
| `runtime/platform_sdl.c` | 1411 | window, screen, input and time on SDL2. |
| `runtime/prepare_run.c` | 154 | the first start of a release (DL_RTSPLIT): the game module is made from the player's DARKLAND.EXE by port/tools/prepare.py (the bundled Python and TinyCC), with a progress window. |
| `runtime/sb.c` | 218 | the digital half of a Sound Blaster Pro: DSP (ports base+6/A/C/E), 8237 DMA controller and the transfer-complete interrupt. |
| `runtime/sound.c` | 200 | AdLib / OPL music through an emulated OPL chip (Nuked OPL3) on SDL audio. |
| `runtime/sounddev.c` | 128 | which sound driver the game gets. |
| `runtime/speaker.c` | 77 | the PC speaker, which the port had not made at all ("PC speaker" in F12 > Settings > Sound played nothing). |
| `runtime/stb_impl.c` | 16 | the stb image reader and PNG writer (public domain, third_party/stb) for replacement art (mods/hdart.c, platform_sdl.c) |
| `runtime/testcmds.c` | 533 | the test commands that look at or change the game's state, for the scripted sessions (port/tools/nctl.py "ucmd NAME:ARGS", release_check.py, monkey.py): a battle at the next card, hits, items, saves, the native screens' actions, dumps of what the game holds ... |

## mods

The game side: hooks into the game's code, its rules and fixes, the models the screens draw from.

| File | Lines | What it is |
|---|---:|---|
| `mods/aging.c` | 139 | the years take their toll in play, as they do while a character is made. |
| `mods/alchemyview.c` | 231 | the alchemy screen as a native screen (nscreen.h; the view is ui/alchemyview.cpp). |
| `mods/ammo.c` | 73 | arrows, quarrels and balls that never run out (assist.infinite_ammo, on by default; the Full Original preset turns it off). |
| `mods/assist.c` | 513 | playing aids of the native port that watch the game through hooks: |
| `mods/autopause.c` | 104 | battles pause themselves (as if Space were pressed) when a member of the party falls, or is badly hurt, or when foes waiting in a castle, dungeon or mine come out to fight (a group of the shown floor's activations steps out: their count, DS:d6cc, falls while the floor stays the same; see mods/foesleft.c), so the player can give new orders in time. |
| `mods/barterview.c` | 299 | the shop screen as a native screen (nscreen.h). |
| `mods/battleend.c` | 102 | the end of a battle said in words (battle.end_words, on by default; F12 > Settings > Battles). |
| `mods/battlerules.c` | 418 | rules of the battles, switches (F12 > Settings > Battles). |
| `mods/battlescroll.c` | 163 | scrolling the battle map at a pace a person can follow. |
| `mods/builtin.c` | 190 | the built-in mods. The same changes as the source mods in mods/ (built into a DOS executable there), done here as run-time hooks on the recompiled game. |
| `mods/cacheview.c` | 288 | the store room as a native screen (nscreen.h; the view is ui/cacheview.cpp). |
| `mods/cardview.c` | 612 | the card screen drawn by Darklands+ itself. |
| `mods/createview.c` | 199 | making a character as a native screen (display.native_create; the view is ui/createview.cpp). |
| `mods/cutcards.c` | 185 | encounters MicroProse wrote and never used, put back (rules.fehde, rules.hospice: F12 > Reconstructed Content, experimental, off by default; the catalogue of what the game holds unused is docs/cut_content.md). |
| `mods/datamods.c` | 409 | mod folders: game files replaced without touching the game's own. |
| `mods/encounters.c` | 1529 | new encounters written as data. |
| `mods/enemyview.c` | 129 | the battle's Enemy Info as a native page (display.native_enemyinfo; the view is ui/enemyview.cpp). |
| `mods/fixes.c` | 2619 | fixes for bugs that are still in the last official version (483.07). |
| `mods/fixgroups.c` | 35 | the fixes (fixes.c) sorted by kind, for the settings' list: from what the game plainly gets wrong to what is open to argument, as players have talked of them over the years (the GOG and Steam forums, the old Usenet posts and the patch notes MicroProse left). |
| `mods/foesleft.c` | 131 | the foes a place still holds that the party has not met yet (battle.foes_left, on by default): shown under the battle margin's enemies, so a castle, dungeon or mine whose fight "does not end" says where the rest still wait (the game ends it only when they are all beaten, or the party walks out and counts as fled). |
| `mods/gamemodel.c` | 367 | the game's state by name (gamemodel.h says what each function reads): where the party, the members, their things and their equipment are kept, and the shared pack's listing and "whom it goes to" (gm_pack_rows, gm_pack_target) for the sheet, the shop, the loot and the store room. |
| `mods/hdart.c` | 1058 | replacement art. A picture file PICS\NAME.PIC can be replaced by art\NAME.png of any size (true colour, with transparency); it is drawn at window resolution wherever the game shows that picture, with the game's own pixel |
| `mods/hdtext.c` | 761 | crisp text. Darklands draws its text with small bitmap fonts (FONTS.FNT; the card font is a 7-pixel blackletter) that players find hard to read once scaled up. Here every string the game draws on the visible page is note |
| `mods/infoview.c` | 125 | the party information screen as a native page (display.native_info; the view is ui/infoview.cpp). |
| `mods/keep.c` | 78 | the player's marks, for each game (party seed) in darklands_keep_<seed>.txt beside the journal (dlp_data_dir): - things marked "keep": never sold (one by one, Sell spare gear) or dropped by the screens of Darklands+, until unmarked; or "sell": sold with Sell spare gear though they are no weapon, armor or shield (the potions the player never wants). |
| `mods/levelwalk.c` | 164 | a click on the level map (Tab, ui/levelmap.cpp) walks the party there. |
| `mods/lodgeview.c` | 528 | staying at an inn, camping, as a native screen (display.native_lodge; the view is ui/lodgeview.cpp). |
| `mods/looks.c` | 1063 | more figures for the party. |
| `mods/loot.c` | 331 | the loot screen and the shops, one at a time no more. |
| `mods/lootview.c` | 276 | the loot screen as a native screen (nscreen.h; the view is ui/lootview.cpp). |
| `mods/losaim.c` | 105 | battle.los_line: while a member is about to shoot or throw at a foe (the pointer on it, order mode), the line the game tests for a clear shot and what stands across it (ui/battleoverlay.cpp draws them). |
| `mods/losses.c` | 146 | what is lost for good is told in a window to click through (the game waits): - an attribute's maximum or a skill lowered, by whatever does it (a card, a curse ...): the members' maxima and skills are watched from one moment to the next. |
| `mods/mapmsg.c` | 73 | the travel map's messages (a robber knight's land, a blighted country ...) as a native box (display.native_mapmsg; the view is ui/mapmsg.cpp). |
| `mods/marchview.c` | 109 | Change Marching Order as a native dialog (display.native_march; the view is ui/marchview.cpp). |
| `mods/memberlooks.c` | 109 | characters keep their figure and colours when they rejoin the party (always, but in the original game), and newcomers at an inn get a look of their own. |
| `mods/menuview.c` | 358 | the game's menu bar (Game, Orders, Attack, Party on the battle field; the menus of the map and the cards) drawn and used by Darklands+ (display.native_menu; the Full Original preset keeps the game's). |
| `mods/musicpack.c` | 438 | music packs: recordings played in place of the game's synthesizer music. |
| `mods/nscreen.c` | 235 | the frame for native screens (see nscreen.h). |
| `mods/offices.c` | 102 | whether a town's task givers have work now, or when again (offices.h). |
| `mods/opening.c` | 117 | the opening (the MicroProse logo, the animations, the music and the digitized speech), F12 > Settings > "At start": the whole opening (the default), its last scene alone (the lady of the Darklands and her welcome), or nothing. |
| `mods/options.c` | 485 | the table of Darklands+'s switches and the presets (options.h). |
| `mods/optwhy.c` | 156 | why each switch exists, one line each: beside what it does (the table's description, mods/options.c), the reason for it, for whoever carries a feature into another engine (the list docs/SWITCHES.md is made from both: DL_DUMP_SWITCHES, runtime/main.c). |
| `mods/orders.c` | 910 | in a battle, a member who is not yet fighting can be sent at an enemy with a click on it (F12 > Settings; the Full Original preset leaves the game's way). |
| `mods/pack.c` | 355 | the shared pack's housekeeping (inventory.shared, mods/sheetview.c): the party's spare things are one pack in Darklands+'s screens, but the game still keeps each thing with a member, and in a battle a bow, a crossbow or a gun shoots only what its bearer carries (can_fire, s014 18bf:0c02: the ammunition of the weapon's kind, far 20a5:7822[type]). |
| `mods/pathfix.c` | 786 | better pathfinding in battles and dungeons. |
| `mods/playlist.c` | 201 | the soundtrack, piece by piece, in F12 > Settings > Sound (ui/settings_setup.cpp): each piece of the game's music (8 ... |
| `mods/politics.c` | 729 | the political campaign MicroProse wrote and never switched on (docs/cut_politics.md), restored as an experimental option (rules.politics, off in every preset). |
| `mods/prayview.c` | 145 | the prayer screen as a native screen (display.native_pray; the view is ui/prayview.cpp). |
| `mods/rosterview.c` | 307 | the party of a new game as a native screen (display.native_roster; the view is ui/rosterview.cpp). |
| `mods/saintinfo.c` | 502 | what each saint's help does in the game's code, beside the description the game shows (DARKLAND.SNT), for Darklands+'s screens (saintinfo.h). |
| `mods/sainttexts.c` | 434 | the "sainttexts" fix: saints whose help, in the game's code, plainly falls short of or departs from their own description (DARKLAND.SNT) do what the description says. |
| `mods/saves.c` | 655 | quick save and quick load, autosaves, and loading any save from the Darklands+ menu. |
| `mods/savesview.c` | 170 | the game's Save and Load as native dialogs (display.native_saves; the Full Original preset keeps the game's). |
| `mods/sfx.c` | 608 | battle sound effects. The battle driver (PSOUND.DB and the others) makes every combat sound on the FM chip or MIDI synthesizer ("random notes rather than clashing swords"). Here they are played as recorded-style sounds i |
| `mods/sharedstore.c` | 224 | one store room for every inn (store.shared, F12 > Settings; on by default): what is left with one innkeeper (a town's inn, a village's old woman) is there at every other, like a chest that follows the party. |
| `mods/sheetview.c` | 480 | a member's sheet as a native screen (nscreen.h; the view is ui/sheetview.cpp, which draws the whole page anew: the doll from the game's pictures, the things, the stats, the party's saints and formulae). |
| `mods/soundpreview.c` | 161 | hearing a sound device before choosing it (F12 > Settings): a piece of music, or ten seconds of a real battle's sounds, played through that device now, then the chosen one back. |
| `mods/teachers.c` | 267 | the teachers a town can offer, and the card options that bring them (teachers.h). |
| `mods/titleview.c` | 134 | the start screen as a native page (display.native_title; the view is ui/titleview.cpp). |
| `mods/travel.c` | 660 | travelling on the overland map by itself, along roads and over bridges. |
| `mods/war.c` | 293 | cities preparing for war, and at war: coded by MicroProse and never switched on (rules.war, experimental, off in every preset; docs/cut_content.md, "Cities at war"). |
| `mods/worldmap.c` | 197 | the whole overland map in one picture (for the world map: M on the travel map or a card). |

## ui

The interface: everything drawn (Dear ImGui), the new screens, margins, maps, settings.

| File | Lines | What it is |
|---|---:|---|
| `ui/alchemydesk.cpp` | 230 | the alchemy screen's workbench (ui/alchemyview.cpp writes on it; where each thing lies: ui/alchemydesk.h): an oak table; the formulary open on it, bound in red leather with gilt tooling and brass corners, its ribbon out of the gutter; an apothecary's chest of walnut, open, 8 x 3 compartments for the ingredients, a label inside its lid; a wax tablet in a wooden frame with its stylus, for the reckoning; a small brick furnace with fire in its mouth and a brass plate for Start. |
| `ui/alchemyicons.cpp` | 346 | the pictures of the potions and of the alchemical ingredients, drawn with ImGui's shapes (no image files). |
| `ui/alchemyview.cpp` | 395 | draws the alchemy screen (mods/alchemyview.c runs it; docs/NATIVE_SCREENS.md) on an alchemist's workbench (ui/alchemydesk.cpp): the formulary open, its left page the index of the formulas the alchemist knows (how many potions the party's ingredients make; a star puts one first, a mark puts it aside, last), its right page the chosen one written out (what it does, its mystic number and risk, its ingredients needed and at hand); the apothecary's chest beside it, a compartment for each of the 23 ingredients with what the party carries, those the formula needs tagged; the wax tablet below: the batches (- and +), the chance for every number of them the ingredients allow (a click there sets it), what may come of the brew and what the chance is made of, in pictures; Start on the furnace. |
| `ui/alerts.cpp` | 152 | news the player must read: a window to click through, the game waiting meanwhile (ui_is_open). |
| `ui/banner.cpp` | 156 | the travel map's banner drawn anew (with the period ornaments, display.ornate): the plaited band across the top (MAPHEADA, 229 x 16 at (60, 0): two strands, a light copper one and a dark brown one, over and under, on grey) and the sky box at its right end (MAPHEADB and its eight frames MAPBRS00..21 at (289, 0): the game shows the hour as a sun eclipsed by a dark disc). |
| `ui/barterview.cpp` | 415 | draws the shop screen (mods/barterview.c runs it; docs/NATIVE_SCREENS.md). |
| `ui/battlelog.cpp` | 351 | what happens in a battle, line by line. |
| `ui/battleoverlay.cpp` | 466 | drawn over a battle: a bar over each fighter (endurance, and strength once it is down by half), who the pointer is on (condition, attack, defence, weapon, and against the party's weapons), and the fast-forward badge (Z). |
| `ui/bestiary.cpp` | 240 | F12 > Appendices > Enemies: every foe of the game (DARKLAND.ENM: 71 kinds of 204 bytes, then 82 named foes of 24 bytes; formats/dlformats/enm.py has the layout) with its attributes, weapon skills, armour (and how hard it is against which weapons), shield and weapons. |
| `ui/cacheview.cpp` | 143 | draws the store room (mods/cacheview.c runs it; docs/NATIVE_SCREENS.md). |
| `ui/cardhints.cpp` | 40 | why an option of a card is greyed: the cards' scripts (src/c) show some options without letting them be chosen - something done already, the time of day, a fact the party has yet to learn, money - and the game never says which. |
| `ui/cardview.cpp` | 819 | draws the card screen (mods/cardview.c runs it): the card's picture stays the game's own (with its palette, frame and any replacement art), the text and the options are set here in the chosen typeface, sized to the text block the card file gives for them. |
| `ui/careericons.cpp` | 93 | a picture for each kind of profession on the making of a character (arms, nobility, church, trade, learning, crafts, common folk), in the manner of the town's pictures (ui/townicons.cpp): a parchment disc, bold shapes with an ink outline. |
| `ui/citynav.cpp` | 1128 | "Go to" in cities. A city in Darklands is a maze of card menus (main street, market, churches, the crafts district, side streets ...); the list here names every place at once, and picking one walks the party there throug |
| `ui/createview.cpp` | 382 | making a character, drawn by Darklands+ (mods/createview.c runs it), over the whole screen. |
| `ui/drawcache.cpp` | 226 | see drawcache.h |
| `ui/drawguard.cpp` | 122 | the last look at a frame before it goes to the graphics card: a triangle no screen of Darklands+ means to draw is taken out, and the first few of them are said in the log ("draw guard: ..."). |
| `ui/editor.cpp` | 168 | the character editor (F12 > Editor, switched on under Helpers): what the save-game editors of the community (Darklands Companion, EDITOR.EXE, CharacterEditor) change, done on the running game: money, names, attributes, skills, saints, formulas, a character's colours (the "garish colors" bug), shield and figure, items' quality and number. |
| `ui/enemyview.cpp` | 131 | the battle's Enemy Info drawn by Darklands+ (mods/enemyview.c runs it): every kind of foe on the field with its picture, how many and how many still fight, its armour, shield and weapons (with the game's word for its skill, but not at Expert), and what each of the party's weapons does against its body armour. |
| `ui/figures.cpp` | 168 | the figure picker of the creation menus: Select Character Image (S) shows every figure at once (the game's own four and the enemies' fourteen, mods/looks.c), each in the colours the member would have; a click (or Enter) takes one, Esc keeps the old. |
| `ui/gamedata.cpp` | 290 | see gamedata.h |
| `ui/gamepics.cpp` | 176 | the game's pictures as textures (see gamepics.h). |
| `ui/guide.cpp` | 165 | WITHDRAWN (not shown since 2026-10-25): ui_guide_tab is called from nowhere. |
| `ui/halloffame.cpp` | 188 | the Honor Roll of Glory, a part of the game MicroProse left unfinished: the picture (pics\hallfame.pic, "Honor Roll of Glory" ... |
| `ui/hourglass.cpp` | 364 | the game's time-passing clock (the "hourglass" of pass_hours_hourglass, s043 150b:2a04), drawn by the interface wherever a Darklands+ page stands over the game's own: the armillary of TIMER01.PIC with the sun-and-moon disc (TIMER03/04/05.PIC, turned by flipping them as hourglass_frame 150b:2d3a does, one of eight positions for each three hours), and the bell, the day and the month in its boxes. |
| `ui/iconsheets.cpp` | 149 | the test command uiconstyles:N: sheets of the pictures drawn with ImGui shapes, to compare them across families and to look at new line art large (docs/REVIEW.md). |
| `ui/infoview.cpp` | 151 | the party information page drawn by Darklands+ (mods/infoview.c runs it): the party's fame and its title, the time and date, where it is and its reputation there, its wealth, and its reputation in every city (sorted by reputation or by name; the city the party is in or nearest marked). |
| `ui/inventory.cpp` | 647 | the things' pictures, drawn by kind (weapons, armour, ingredients, potions ...) for every screen that lists them; what a thing is good for (ui_item_details: the numbers of a weapon, armour or shield and who in the party could use it); the tidy order of a pack; and giving a thing to another member through the game's own routine (give_stack, s031 18e7:007c: the test command ugive). |
| `ui/itemedit.cpp` | 137 | F12 > Editor > Items (for a mod): the item list (DARKLAND.LST: 200 records of 46 bytes, then the saints' and formulas' names) as an editable table. |
| `ui/journal.cpp` | 1186 | the town journal: as the party travels, it notes the towns it reaches (with the shops each one has, from DARKLAND.CTY) and the full stock of every shop whose wares it looks at, with qualities and prices. |
| `ui/levelmap.cpp` | 466 | Tab in a battle or while exploring a castle, mine or dungeon (the tactical view, which shows only 14 x 10 cells around the party): a map of the level as far as the party has seen it, in the view's own slant, with the party, the enemies in sight, doors and furniture, and at its top right the floors in short (a square a floor, the stairs seen between them). |
| `ui/libraries.cpp` | 126 | see libraries.h. Which places have a library the party can study in: the saint card ($Learn00, card 0x39) is reached from a town's monastery ($CityM00), its university ($Unive00) and a monastery in the country ($Count04) |
| `ui/lodgeview.cpp` | 404 | an inn's room or a camp drawn by Darklands+ (mods/lodgeview.c runs it): the members with each one's day, what the chosen member can do (the original's words; greyed with the reason where not), the town's teachers with their fees, what a day costs against the purse, and Spend a day, Until rested and Leave. |
| `ui/lootview.cpp` | 167 | draws the loot screen (mods/lootview.c runs it; docs/NATIVE_SCREENS.md). |
| `ui/mapmsg.cpp` | 43 | a message of the travel map (mods/mapmsg.c runs it) in the cards' style: the text in the game's typeface on parchment over the map, and what goes on. |
| `ui/marchview.cpp` | 90 | Change Marching Order drawn by Darklands+ (mods/marchview.c runs it): a dialog with the members in their order, each with its figure and a handle to drag it to its place; Done keeps the new order, Cancel the old. |
| `ui/margins.cpp` | 993 | on a wide screen the 4:3 picture leaves dark margins; they can show what the game otherwise keeps behind menus: the date and hour, where the party is, the purse, the tasks the party has taken on (with their deadlines), and on the other side the town the party is in, the blessings and potions working on the party, and who carries too much. |
| `ui/margins_battle.cpp` | 402 | the margins in a battle: the right margin's fighters, their orders as buttons (with the potions' pictures), the enemies still waiting; the order buttons at the foot of the field when the window has no margin. |
| `ui/marks.cpp` | 73 | small signs drawn with ImGui shapes (marks.h) |
| `ui/menuview.cpp` | 172 | the game's menu bar drawn by Darklands+ (mods/menuview.c runs it): the bar over the top of the picture, the open menu under its title, a sub-menu beside its item. |
| `ui/money.cpp` | 189 | see money.h. |
| `ui/niche.cpp` | 272 | the character sheet's stone niche drawn anew (with the period ornaments): the game's ARMSBACK.PIC (136 x 170) at the window's resolution. |
| `ui/nsdraw.cpp` | 543 | see nsdraw.h |
| `ui/optpreview.cpp` | 185 | small pictures of what a switch in the settings turns on, up to four in a row under the switch, each with a word (ui/settings.cpp option_row). |
| `ui/ordericons.cpp` | 258 | the pictures on the battle's order buttons (ui/margins.cpp: the game's Orders and Attack menus as buttons): a sword for Std Attack, a shield for Parry, footprints for Walk towards, a white flag for Surrender ... |
| `ui/ornament.cpp` | 545 | the period ornaments (ornament.h) |
| `ui/overlay.cpp` | 811 | things the port draws over the running game when the Darklands+ menu is closed: the chances of success next to card options (mods/assist.c computes them). |
| `ui/overlay_text.cpp` | 530 | crisp text over the game's own screens (mods/hdtext.c finds the game's text runs; here they are drawn again at the window's resolution in the chosen typeface, with the abbreviations spelled out and the skills' pictures), the chances beside the original cards' options, and the pointer drawn live. |
| `ui/padglyphs.cpp` | 180 | a game controller's buttons drawn on screen (padglyphs.h) |
| `ui/panelcheck.cpp` | 72 | the checks, every frame, that no two of Darklands+'s panels lie over each other, and that the game's hand never shows over a Darklands+ window (two pointers). |
| `ui/party.cpp` | 30 | a short word on what each attribute and skill is for (the character sheet's Skills page shows it under the pointer, with the whole party's values). |
| `ui/partypanel.cpp` | 528 | the party panel drawn by Darklands+ (display.native_panel; the Full Original preset keeps the game's). |
| `ui/pointer.cpp` | 115 | the game's pointer drawn anew (display.pointer_drawn): its armored gauntlet in plate, lames and rivets, lit from the upper left, drawn at any size in place of the game's little sprite (COMMONSP.IMG's first, cursor_default; in a battle the two of BATTLEGR's, unpaused and paused). |
| `ui/prayview.cpp` | 146 | the prayer screen drawn by Darklands+ (mods/prayview.c runs it): the saint's statue, legend and what the saint does, the favor to spend with - and +, the chance of success, and Pray (for a member, when the saint helps one) or Don't pray. |
| `ui/professions.cpp` | 377 | see professions.h. |
| `ui/pstone.cpp` | 246 | the Philosopher's Stone, as the alchemists pictured it (display.pstone_look). |
| `ui/questicons.cpp` | 207 | a picture for each kind of quest (ui/quests.h QuestCat), drawn with ImGui's shapes on a small parchment roundel, in the manner of a woodcut or a guild's sign: a great helm before crossed swords for a robber knight, a purse for a reward waiting, the miners' hammer and pick (Schlägel und Eisen) for a mine's trouble, a horn for the Wild Hunt, a lowered portcullis for a town that bars the party, the pillory where the watch wants it ... |
| `ui/quests.cpp` | 643 | the quest list (on the world map, M): the game keeps its quests as timed events (far table 2e38:1b2c, 300 pointers to 0x30-byte records) but never lists them; players used MicroProse's own DKQUE.EXE or other tools for that. |
| `ui/report.cpp` | 238 | "Report a problem" (F12 > About): what a bug report on GitHub needs, in one zip - this session's log and the last one's, the settings, the two newest saves (each .SAV with its .BSV) and a short summary (the build, the system, the preset) - the user's own name taken out of the paths in them; then the zip's folder and GitHub's bug form opened, the form's version, system and preset filled in (.github/ISSUE_TEMPLATE/bug_report.yml, its fields by id). |
| `ui/repwatch.cpp` | 65 | a town's opinion of the party (its reputation, location +0x12) watched while a card or the map is up: each change is remembered with what was going on when it came (the card, the option just chosen on it, the date), flashed beside "Reputation" in the right margin (ui/margins.cpp) and listed in its tooltip, and written to the log. |
| `ui/richtext.cpp` | 235 | text with the attributes' and skills' shields in it (richtext.h): the text cut into words and shields, laid out a line at a time like ImGui's own wrapped text, each shield the size of the type's capitals (UI_SHIELD_W wide), its value after it; shields side by side (a group sharing one value) joined by a bracket under them; a member's head ({@m}) square, the type's height. |
| `ui/rosterview.cpp` | 259 | the party of a new game, drawn by Darklands+ (mods/rosterview.c runs it). |
| `ui/rules.cpp` | 263 | game rules as data. The weapon numbers live in tables of the game's code (far segment 20a5, read by the battle module s014: skill used, time for a blow, hands and penetration, damage, skill and strength wanted, reload ti |
| `ui/saints.cpp` | 243 | the saints as Darklands+ tells them (the character sheet's Saints, the prayer screen, the tooltips): where a description's legend ends and its promise begins, the game's description with what the help really does (mods/saintinfo.h), whether encounters call on a saint; and the blessings and potions at work now and how long they last (effects_now: the margin's "In force"). |
| `ui/saves.cpp` | 123 | the saves' preferences (F12 > Settings); the old list of saves (tab_saves, no longer a tab: the game's Load does it), the party in it and its comment, newest first; load or delete any of them; quick save / quick load and the autosave settings (mods/saves.c does the work through the game's own routines). |
| `ui/savesview.cpp` | 171 | the game's Save and Load drawn by Darklands+ (mods/savesview.c runs them): the comment to save under, and the saves there are, newest first, each with its comment, where and when in the game, who, the purse and how long ago it was written. |
| `ui/screenpics.cpp` | 210 | pictures of the new screens from the player's own game, for the settings' "New screens" switch: the first time a card, a shop, a character sheet, the inn and the alchemy screen have stood still on screen for a moment (no window of Darklands+'s over them), the whole window is kept, small, in <data>/previews/<name>.png. |
| `ui/settings.cpp` | 602 | the Darklands+ window's Settings page (F12), and its Mods and Reconstructed Content tabs. |
| `ui/settings_look.cpp` | 274 | the Settings page's sections on what things look like (ui/settings.cpp draws the page): Text (the original lettering or the new text, the typeface, the coins) and Frames and clock (the band around the windows, the frames, the buttons, the clock, the potions' grade marks). |
| `ui/settings_setup.cpp` | 422 | the Settings page's sections on the game on this computer (ui/settings.cpp draws the page): Screen, Intro, Sound and Controller. |
| `ui/sheetview.cpp` | 1543 | draws a member's sheet (mods/sheetview.c runs it; docs/NATIVE_SCREENS.md), the whole page anew beside the party panel. |
| `ui/skillicons.cpp` | 538 | the skills', attributes' and F12 tabs' pictures, drawn with ImGui shapes so they read at any size (the character sheet, character creation, the roster, card chances, the tabs). |
| `ui/teachersui.cpp` | 143 | the teachers a town can offer, as shown (ui/teachersui.h). |
| `ui/tipplace.cpp` | 144 | where every tooltip goes: beside the thing it describes, never over it nor over the pointer - below it, or above when there is no room below, else at its side, and inside the window. |
| `ui/tips.cpp` | 155 | the first words of advice, each in a window the game waits behind (ui_is_open): at the first battle of a session with Improved combat, how the party fights now (the start screen's note pointing to the New Player Guide went with the guide; the welcome, the very first time, stays). |
| `ui/titleview.cpp` | 110 | the start screen, drawn by Darklands+ (mods/titleview.c runs it): laid out as the game lays out a card, the words on the parchment at the left and a picture at the right (the castle at dusk the game shows on the road to a town, pics\aproch1.pic), the Darklands lettering of the original start screen (pics\startscr.pic) cut from its stone. |
| `ui/townicons.cpp` | 384 | see townicons.h. Drawn with ImGui's shapes (pen.h) in the manner of the quests' pictures: bold shapes with an ink outline, readable at the size of a line of text. |
| `ui/travel.cpp` | 37 | the route of auto-travel drawn over the overland map (mods/travel.c walks it; the world map, ui/worldmap.cpp, is where a journey is chosen). |
| `ui/ui.cpp` | 769 | the native port's own interface, drawn over the game: the Darklands+ window (F12) and its pages (the settings pages in settings.cpp), help, notices. |
| `ui/ui_windows.cpp` | 443 | the interface's side windows: About, the welcome on the first start, the news of a new version, getting the MT-32 working, and F11's keys and clicks for the screen in view. |
| `ui/worldmap.cpp` | 1113 | the world map: the whole of the Holy Roman Empire as the travel screen draws it (mods/worldmap.c), to pan and zoom, with towns and places named, the party, the route of auto-travel and the places the quest log points to. |
| `ui/worldmap_draw.cpp` | 585 | the world map's pictures (the places' seals and painted icons, the frame, the compass rose, the boats' red ink and ships, the layer buttons' glyphs, a quest's halo), its list of places, the names over the game's own travel map, and a test sheet of the pictures. |

## tools

Tools: tests, checks, the harness.

| File | Lines | What it is |
|---|---:|---|
| `tools/aiart.py` | 610 | repaint the game's pictures with an image model, as a replacement-art mod. |
| `tools/arena.py` | 313 | a combat test arena: battles set up to push Improved combat (battle.improved, mods/orders.c, mods/battlerules.c) to its limits, run in the game (nctl, X11), logged and checked. |
| `tools/artpack.py` | 204 | make a replacement-art mod from the game's own pictures. |
| `tools/battletrace.py` | 156 | stress tests of battle movement (pathfinding in a melee crowd), on your own game files. |
| `tools/bin2c.py` | 13 | bin2c.py OUT.c NAME=FILE ...: embed files as const byte arrays (NAME, NAME_SIZE). |
| `tools/check_docs.py` | 194 | what the player-facing texts say against the code, so that no text drifts from it. |
| `tools/check_switches.py` | 24 | check_switches.py OLD.md NEW.md - fails if a switch key in OLD (docs/SWITCHES.md as committed) is missing from NEW (as the game now writes it) without being listed under "Retired keys" there: a switch's key is its name for good (saved settings, presets and other engines know it by it). |
| `tools/code_map.py` | 38 | writes docs/CODE_MAP.md: every source file of the port with the first sentence of its header comment (each file starts with one saying what it is), by folder, with its size. |
| `tools/copy_catalog.py` | 61 | every text a player can read that Darklands+ itself writes (not the game's own), by file and line: docs/COPY.md. |
| `tools/encfuzz.py` | 48 | encfuzz.py ENCCHECK [SEED] [CASES]: feeds build/enccheck (port/tools/enccheck.c, built with -fsanitize=address,undefined) mutated copies of the shipped encounter scripts; a batch whose run fails (a sanitizer report, a crash) is kept in encfuzz-crash/ with its log. |
| `tools/gen_gamevars.py` | 75 | port/runtime/gamevars.h from the decompilation's names. |
| `tools/gotosweep.py` | 44 | Go to (G) to every place of a town and on from there, in a running game: the walk must arrive or stop for a reason the player is told (shut at night, not in this town), never fail for want of a way ("not found in this city", "cannot find the way", "the way is blocked"). |
| `tools/make_icon.py` | 40 | make_icon.py OUT.ico [OUT.png] - the Darklands+ icon: a red shield with a gold rim and a Fraktur "D+" (DL Fraktur, SIL OFL). |
| `tools/monkey.py` | 127 | monkey.py NAME MINUTES --game DIR [--seed S] [--x11] [--exe EXE] [--load SAVEDIR:FILE]: soak test. |
| `tools/nctl.py` | 197 | interactive control of a headless native session (for automated play that needs to look at the screen between steps, e.g. |
| `tools/nfight.py` | 76 | nfight.py NAME [--max N]: in an nctl --x11 session on the travel map, travel about until a battle starts: on each card the option that leads to a fight is clicked (read from the crisp text runs), other cards are passed with their last option. |
| `tools/nhunt.py` | 19 | nhunt.py NAME [--max N] [--pts x1,y1,x2,y2]: travel back and forth on the overland map of an nctl session until something other than the map shows (an encounter card). |
| `tools/nplay.py` | 133 | run a tools/play.py-style script against the NATIVE build (headless) and save screenshots as PNG. |
| `tools/package.py` | 151 | the Windows release of Darklands+: only our own code and the tools that make the game module on the player's machine (runtime/split.h). |
| `tools/perfcheck.py` | 72 | how long the interface takes to draw on each screen (run after every new feature). |
| `tools/prepare.py` | 137 | make the game module (runtime/split.h) from the player's own DARKLAND.EXE. |
| `tools/release_check.py` | 151 | a scripted session through the parts of Darklands+ that a release must not break, in a window (native screens are drawn only there), on your own game files. |
| `tools/reloc_check.py` | 54 | every segment the DOS loader would have fixed up in DARKLAND.EXE's code, and whether the translation (port/recomp/recomp16.py) adds the load segment there too. |
| `tools/smoke.py` | 52 | quick checks of a Darklands+ build against your own game files, before a release. |
| `tools/soundpack.py` | 313 | build the sample battle-sound pack from free recordings. |
| `tools/uitour.py` | 70 | every tooltip beside what it describes (never over it nor over the pointer), and no two of Darklands+'s panels over each other. |

