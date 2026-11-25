# The review pass

A pass over the whole of Darklands+, run from time to time, for three standing aims: code a newcomer can find
their way in, one style in words and pictures, and fewer, plainer switches. It looks at the whole before the
parts: a pass that only works through lists finds what is wrong with each item, not what is wrong between them
(two pictures for one thing, a text that a later change made untrue, a switch that only matters with another).
Every pass does all of it; each ends with a line in the log at the bottom, so the next one knows where the
last one stopped and what it missed.

## 1. The whole, first: walk it as each kind of reader

Play through, as a player would, with the one-time windows shown (the test sessions hide them: start with
`DL_TIPS=1 DL_WELCOME=1 DL_MT32_PROMPT=1`), and take a screenshot at each step:

- **A new player**: the first start (the MT-32 window, the intro, the welcome on the start screen, the guide's
  note), a new party, the first town, the first card, the first battle (its tips), the loot, the inn, travel,
  saving, quitting. Is every step explained where it happens, once, in plain words? Is anything said twice in
  two ways?
- **A player of the original** on the Full Original and Purist presets: does anything of Darklands+ still show?
- **A returning player** of an earlier version: does anything they knew move without a word?
- **A maintainer** opening the code for the first time (docs/DEVELOPERS.md, CODE_MAP.md): can they find where a
  thing on screen is drawn, and the switch and the reason for it, in a few minutes?
- **A modder** (docs/MODDING.md): does the example mod still work as written?

For each screen ask: does every thing on it have one name, one picture and one home (the place where it is
explained and switched)? Does the text match what the game does now?

## 2. Then the sweeps (each over all of it, never a sample)

**Code.** `python3 port/tools/code_map.py > docs/CODE_MAP.md`; a file past about 1,500 lines that draws several
unrelated things is split by what it draws; every file starts with a header saying what it is and who calls
it. Each switch has its reason in `mods/optwhy.c`, each fix its group in `mods/fixgroups.c`; regenerate
docs/SWITCHES.md (`DL_DUMP_SWITCHES=docs/SWITCHES.md`) and run `tools/check_switches.py`.

**Pictures.** The icon sheets (test commands `uiconstyles:2` and `uiconstyles:3`) show every family: town, quest
and career icons, F12 tabs, Settings sections, the inn's day choices, battle orders, ingredients, potions,
the world map's places and layers. A family drawn anywhere but missing from the sheets is added to them first.
Then:
- one picture, one meaning, compared **across** families, not within one;
- the default is a color picture with no background (as the F12 tabs); a frame (roundel, shield, seal) only
  where pictures of different families are on screen together and must be told apart, and then one frame per
  family on that screen;
- within a family: the same size, line weight, outline ink and color rule (the ingredients are the model);
- one value for each shared color (the lectern ink, the blue of Darklands+ additions, the rubric red);
- a sign (the ingredients) is abstract, made of the tables' vocabulary (rings, strokes, crosses, forks,
  triangles), not a small drawing of the thing; taken from the period's tables where one exists (looked up
  first, never invented while a real one is there);
- new line art looked at large (uiconstyles:7, 8, 1000+id): one stroke for every line that turns a
  corner (no notches), curves smooth (no facets), ends cut square, gaps even.

**Layout.** In F12, a section with a long list (a table of weapons, of enemies, of items, a chronicle) is a
tab of its own; short sections among other short ones are a list at the left that jumps to them (as
Settings and the New Player Guide).

**Words.** `python3 port/tools/copy_catalog.py > docs/COPY.md`, read as a player who never played the original:
literal verbs (what really happens), no technical talk, jargon explained once where it first appears, names
that say what is inside, the exact F12 paths, American spelling, "world map (M)" / "overland view" / "level
map". For every switch removed, merged or renamed since the last pass: search every text (COPY.md, the guide,
the welcome, the tips, README) for anything that still calls it optional or uses its old name. Long lines are
wrapped (ImGui's BulletText and TextDisabled do not wrap).

**Pointer and tooltips.** One pointer at a time: over a window of Darklands+ the system's, and the game's hand
not drawn (`windows_up` in ui/ui.cpp lists those windows: a new one goes there). Where the hand is drawn, its
point is where a click lands: with `DL_LIVE_POINTER=1`, `uhover:x,y` and a window shot, the hand's corner sits
on x,y. Every tooltip that belongs to a thing on screen opens beside it, never over it nor over the pointer
(`ui_tip_beside`, ui/tipplace.cpp; the item tooltips' `show_tip` in ui/overlay.cpp; the battle's
`place_by_pointer`); a hand-placed `SetNextWindowPos` before a tooltip is a finding.

**The New Player Guide** (ui/guide.cpp) is withdrawn: reading the code was not enough (its first line, a new
party's 200 florins, came from MicroProse's "Initial Card for Testing Only"). It returns only when every
statement has been seen to hold in a test session of the game itself, the session's log kept beside it.

**Switches.** Ask of every switch in docs/SWITCHES.md:
1. Would anyone ever turn it off? If not, it is always on (retire the key in `mods/optwhy.c`).
2. Is it useless without another? Then it sits under that one, or both become one.
3. Do some always go together? Then one switch for the group (a bundle in `mods/options.c`).
4. Does its name say what the player sees? Does its picture show what it turns on?

**Behavior.** The combat arena (`tools/arena.py`) and `tools/perfcheck.py` (within budget).

## 3. Before the pass is done

Build, the screenshots of what changed looked at, docs regenerated, and a line added below: what the pass
covered and found, and what it could not check.

## Log

- 2026-10-24 (morning): F12 tabs given icons of their own (they borrowed skill pictures); the switch table made
  complete. Missed: pictures repeated across families (compared only within one), the inn's new pictures not
  checked against the others (they were not on the sheet).
- 2026-10-24 (afternoon): pictures compared across every family: inn, reward, trade, Latin, Streetwise,
  Charisma redrawn; Settings split in three files; two bundles. Missed: the one-time windows (hidden in test
  sessions): the welcome still said every addition could be switched off; its keys ran off the window's edge.
- 2026-10-24 (evening): words searched for retired names: "Play the original game" (gone since the presets) was
  still in file headers, NATIVE_SCREENS.md and port/README.md, and the players' readme in the zip still named
  "Buggy Purist", "Easy" and a Helpers tab; Fame's new picture repeated Speak Common's trumpet (caught only by
  the large sheet, uiconstyles:8: compare new pictures there, across families); the ingredient signs' corners
  notched (strokes drawn apart: look at new line art large, uiconstyles:7). Open: whether the Journal, New
  Player Guide and Editor keep sub-tabs (asked). Not checked: an Ironman save deleted at the party's death
  (no death screen could be brought about in a test).
- 2026-10-24 (night): the card's member heads showed the figures' backs (SMALL's front is its right half);
  the shields on a card overlapped (a shield is 1.25 sizes wide, they were set 1.1 apart); tooltips at the
  window's right edge ran off it. Pictures settled: Agility steps (close to the battle's "use stairs" order:
  asked), Charisma a rose, Latin a column, Read & Write a quill on a sheet, Stealth the mask, Streetwise a cat,
  Reputation the medal, Fame the trumpet, Speak Common a speech bubble. A triskelion tried for Agility was
  dropped (it reads as an extremist emblem): check a new emblem for that too.
- 2026-10-24 (RC23, then a pass): the ingredients' old pictures removed now the signs are settled (the four
  bases moved among the signs); the icon sheets (test only) moved out of skillicons.cpp into iconsheets.cpp;
  the build's one warning fixed. Settled since the last pass: F12's sub-tabs (Journal and Editor tabbed, the
  Guide a list at the left). The arena's misses were its own: a shot's draw and loosing between two samples
  (now counted from the battle log too), a held member "walking" toward a foe out of reach (the game's Hold),
  the seeds' runs of 'original' not exempt as the first.
- 2026-10-25: from a player's reports on RC23. The New Player Guide audited statement by statement against the
  code (clothing not sellable, the watch, camping's risk, saving, prices, armor, saints, which places most
  towns have); the town icons cut to what most towns lack (counted: `utownoffers`); one pointer over
  windows (the tips window drew the hand too, off the click point); tooltips beside their thing; skills and
  attributes heraldic everywhere; the sheet's hidden game text no longer drawn. Not checkable here: the music's
  timing on a real sound card and display (this machine renders in software); the CM-32L's battle sounds
  differ from the MT-32's only slightly in the recordings (its settings line said "chimes and odd noises": now
  "some battle sounds a little different").
- 2026-10-25 (afternoon): after a player's reports on RC26. Fixed on the way: the battle tooltip's melee weapons
  (it read the combatant's +0x23 since 1.2; the weapon is +0x51), a false "Lost for good" after every battle for a
  member with a saint's boost to a skill (the events are out of memory at a battle's end), horses on the sheet
  (a spare one could not be given or ridden; a drag from the pack fell through as a click on the party panel).
  Words: the 1.3 changelog cut to what changed since 1.2; "Std Attack" left only where the game's own menu says it
  (Darklands+'s button and texts say Standard); the MT-32 window no longer calls the CM-32L chimes. Pointer: the
  order strip's tips placed by ui_tip_beside (a hand-placed position had crept in). Code map, COPY.md and
  SWITCHES.md regenerated; check_switches OK (212, 20 retired). Full Original: the battle has no order strip or
  margins; looks (text, ornaments) stay as chosen, as designed. perfcheck at 2560x1440: build time within budget
  everywhere (worst 6.9 ms), but vertices over the 60 000 mark on the sheet (78k), the world map (90k) and alchemy
  (104k); the ornaments are 62k of alchemy's 89k (display.ornate off: 27k). Then done: the plaited bands made into
  pictures by the processor (ui/drawcache.cpp, mode 4): perfcheck at 2560x1440 now 6k (card) to 42k (alchemy)
  vertices, every screen under 60k; a picture costs up to ~70 ms once at 4K (spread, ~12 ms a frame). Not checkable here: Go to University's "cannot find the way from this card" (not reproduced from the
  player's save at any of 30 places, nor after the university's own cards; the log now names the card, and the last
  session's log is kept as darklands_port_previous.log).
- 2026-10-25 (evening): optimization pass. Measured with perfcheck (2560x1440), strace (the system calls of each
  screen, 3 s each: tools in the scratchpad's fsprof.sh) and gdb stack samples of the main thread. Found and fixed:
  settings looked up along a list of ~150-300 keys thousands of times a frame (an index now: interface build time
  down a third to a half); a battle reading overlay code from DARKLAND.EXE ~3,300 times a second (each section
  read once and kept: 20,670 system calls in 3 s of battle to 885); after each card-chance evaluation every open
  file sought back whether moved or not; the portraits folder checked 40 times several times a second; the plaited
  bands as pictures (above). Every screen within budget now (build 0.3-1.1 ms average, at most 41k vertices,
  85 draw calls). Left as they are: the card chances re-evaluated once a second (a 1.1 MB copy each, ~2 ms/s);
  large sparse drawings (the alchemy desk, the shop counter: 33-41k vertices) kept as triangles, since a
  processor-drawn picture of them costs ~170 ms at 1600x900; the opening's PAN files left open by the game when
  a key ends them (the original's own; 64 handles to spare). Startup to the title is mostly the game's own waits.
- 2026-10-25 (night): the problems that kept coming back, looked at for their common cause rather than one by one.
  Go to's wording (each report a phrase missing from a table): Go to now reads the card's own text from memory,
  and two tests cover it all - every line of every street card matched (`ucitylines`, in release_check) and every
  place of a town walked to from a save (tools/gotosweep.py: 23 of 23 from the player's DKSAVE85). Raw byte
  offsets into the combatant records (the battle tooltip's wrong weapon read +0x23): named fields (runtime/game.h
  CB_*), the machine code unchanged. Tooltips over their thing or the pointer, fixed one screen at a time: every
  tooltip is now placed by one rule (ui/tipplace.cpp, also those ImGui placed by itself), a native screen's hit
  areas tell it where the thing is (NsDraw::over), the pointer's real size is used (the game's hand, from its
  sprite), and each frame a check logs a tooltip over the pointer or its thing, panels lying over each other
  (ui/panelcheck.cpp) and the game's hand over a window of Darklands+ (two pointers); tools/uitour.py rests the
  pointer on a grid over every screen and fails on any of them. The sheet's own 650 ms tooltip wait (on top of the
  Tooltip delay) removed. The shared pack's per-screen copies (four listings, three "whom it goes to") made one
  (gm_pack_rows, gm_pack_target). The texts that say the same things (README, changelog, release notes, the
  download's README, MODDING.md, the welcome) checked against the code by tools/check_docs.py (it found the
  download's README still naming "F12 > Settings > Display" and an MT-32/CM-32L switch that is gone). A battle's
  memories: the route planner's now forgotten with the rest at the next battle (the shooters' "shoots" that never
  came had the same cause). Proposed, not done: the combat behaviour between orders made one ordered list of rules
  per member, in one file, with one memory cleared in one place (now: two hooks on the same game routine in two
  files, 19 per-member memories in three).
- 2026-10-26: the inn's wash on players' Windows machines (text in the colour of the page's ink over the whole
  window, then, with RC30's draw guard taking those letters out, the buttons blank). Not the renderer (OpenGL and
  Direct3D 12 the same), not the save, not the drawing cache; the Windows program under Wine showed it, the Linux
  one never. Each text of the inn logged with the vertices it made: the teacher rows' text at y 5630. The cause:
  the skill icon's shield (ui/skillicons.cpp shield) put 13 corners in a list of 12; the 13th went onto the slot
  where Windows x64 keeps the caller's xmm6, which held the inn's row position (later its text size), so these
  became the icon's x. Linux keeps no registers there. Fixed in 1.3-RC31. What would have caught it here: an ASAN
  build (make ASAN=1) stops on it at the roster, the first screen with a shield ("stack-buffer-overflow ...
  shield"). tools/uitour.py now fails on a memory error or a game that has stopped (before, a dead game passed
  every screen); run it with the ASAN build before an RC. Run so on 1.3-RC31: every screen and the inn with the
  player's save clean. The draw guard stays, as the net under a slip of this kind.
- 2026-10-26 (morning): the standing orders run through after RC31. The Windows compiler with every warning on
  (-Wall -Wextra -Wformat=2) over runtime/, mods/ and ui/: nothing; the shield's kind of slip is ASAN's to find
  (uitour with the ASAN build, now in docs/RELEASING.md's step 0 with every other check). Looked back at the
  wash's earlier diagnosis: twice (commits of 10-23 and 10-25) pictures drawn into on the graphics card (display.draw_cache 2) were
  blamed for it and taken out of the default; the wash came from the inn, the camp and the margins, the screens
  that draw shields, so it was the shield all along. Mode 2 draws the inn as the others do now (under Wine); its
  description no longer says it washes the screen, and drawcache.cpp says what happened. The default stays the
  processor-drawn pictures (lighter anyway). Made from the code and now checked so: docs/COPY.md had drifted by
  hundreds of lines, docs/SWITCHES.md by three descriptions; check_docs.py reports a stale CODE_MAP.md or COPY.md
  (--fix writes them) and a source file without a header (two had none: gamemodel.c, sheetview.cpp), and
  release_check.py writes the switch list and reports when it changed. Words: COPY.md's Terms hold everywhere
  (no "in use" for equipped, the attributes capitalized); the shop's hint ends as the other item screens'.
  Not split: mods/fixes.c (2,577 lines), one table of independent fixes, each described in the header.
- 2026-10-26 (later): the player's reports on RC31. Go to failed again ("cannot find the way from this card",
  Freiberg's main street read as block 4 of four): the cause of the whole run of such reports was the variable
  Go to read for the option list's text block, options_block (DS:ee53), which only some scripts set - another
  card's value stayed in it. The text block on show is shown_block (DS:e959), set by card_text and card_options
  both; Go to reads that now, and the earlier fallback ($SIDES01 "a block that has no options: take one that
  has", the same cause) is gone. gotosweep from the player's DKSAVE90: every place of Freiberg. The cave south-east
  of Basel: the offmap fix put it on row 930, which a step never reaches (the marker stops at screen y 0xb1);
  now 929, moved before the overland lists its icons. The battle view dragged: edge scrolling off while
  dragging (the view ran off by itself), and the last whole frame kept until the people are drawn again (they
  flickered: a recording of a drag had frames with nobody on the field). Card chances one look everywhere (the
  alchemist's formulae showed every member's chip). gamevars.h had three names added by hand, which the next
  run of its generator would have dropped: made from the headers again, and check_docs.py checks it.
- 2026-10-26 (evening, 1.4 under way): the release check's inn step failed three runs running (Go to Inn "not
  found": the market's options read empty). Go to read the card's text from the game's buffer (msg_file, DS:a895),
  which the loot, a level's enemy table and screens opened from the menus share: after Party Info and the
  marching order it held another text. Not patched with a fallback this time (the $SIDES01 lesson): the text is
  now copied as it is loaded (msg_load hooked, ui/citynav.cpp card_blocks), and Go to never reads the shared
  buffer. One name for what was written as numbers: the attributes and skills (gamemodel.h ATTR_*, SKILL_*; the
  teachers' own SK_ enum and the card's VIRTUE = 9 gone); the Philosopher's Stone's drawing declared in one header
  (ui/pstone.h, was three copies, one reading the stone at a raw 0x9078); the alerts' declaration (ui/alerts.h, was
  three) and their rows of shields; the alchemy screen's signs (star, put aside, pentacle, the batches' bars, the
  ingredients lost, a stink, an explosion) moved to ui/marks.cpp and onto the icon sheet (uiconstyles:2) with the
  age's hourglass. The teachers drawn once (teacher_row) for the margin's tooltip and the map's panel (the margin
  had a text copy). A town's Philosopher's Stone worked out from the cards' own rule (town_stone_offer) instead of
  noted from the card's words: the journal's alchemist facts dropped. Dead code the compiler's -Wall named, gone:
  the margin's old teachers lines, two clock plates and a throne never drawn, unused helpers and variables in nine
  files (orders.c's "engaged", left from an older rule, computed and never read).
- 2026-10-26 (night, 1.4): the shields pass. Shields were drawn by hand beside text in each screen; now one
  markup (ui/richtext.h: {Strength}, {Strength:30 of 35}) goes through every native screen's text and
  tooltips, the margins' tooltips, notices, the battle log and ImGui text, and check_docs fails on a name no
  shield has. One key for every shield (ui_key_icon / ui_key_name / ui_key_of: attributes, skills, the town's
  opinion, fame, age) in place of three numberings. The Saints page's filter searched the account's text a
  second way beside the chips' parse: it now reads the chips (saint_effect), so the filter, the chips and the
  order agree; usaintchips (in the release check) fails on any part of a saint's help the parse does not know.
  The fade of a drawn picture (ui_dim_since, two copies) and the potion grade's chevron badge (ui_mark_chevrons)
  shared. Found on the way: the pointer's crossed blades never showed (the game sets cursor_target only while
  the button is held): it now takes the orders' own pointer test. Left as words: the switches' and built-in
  mods' descriptions (they also make docs/SWITCHES.md), and the sheet's Skills page and character creation,
  where each shield's name is learned.
- 2026-10-27 (1.3.1 and 1.4): the translation, checked as a whole. The dwarves' door (GitHub issue 2) came from
  one gap in recomp16.py: a relocated immediate of cmp was translated without the load segment (only mov and
  push had it), so the game's comparisons of stored routine pointers never matched: a card's opened wall, the
  battle save's routine numbers (every trigger came back a pit) and the traps enemies spring. Fixed at that one
  place (imm_val) rather than at the 13 sites, and tools/reloc_check.py (in the release check) now lists any
  relocation in the program the translation does not add the load segment to: none left but one word of data
  after the runtime's exit call. Old battle saves are put right as they load (saves.c), not by a second rule in
  the game's restore. The pathfinding's "no way after eight asks" had grown three exceptions in a day; they are
  one account now (pathfix.c). Edge scrolling held the frame only for drags: the hold moved to the view's redraw
  itself (battlescroll.c view_redraw_hook), so every way of scrolling gets it. Saves past 99: the DOS layer's
  8.3 rule hid them; one alias (dos_name_alias) at path resolution, one reader of the save in play
  (saves_cur_name) in place of seven.
- 2026-10-28 (1.4): the combat AI given a shape. Between orders the members' behaviour had grown as switches
  and exceptions (a bow-or-sword toggle, the player's melee remembered in one array, "by our doing" in another,
  a rescue check inside the bow's return): it is now one idea, each member's way of fighting (styles.c: eight
  ways, kept by name), with an order of the player's taking the way's place for the rest of the fight (one
  array, ov), and each step of the round (idle shooters, idle fighters, the stale foe, the bow's return, the
  stances) asking only the way. It moved out of orders.c (the clicks) into ways.c. docs/MERGING.md's switch
  table, written by hand at 1.2, had 17 switches since retired and lacked 125: it is now made from the switches
  and the sources (switch_map.py, by check_docs --fix), and its classes and presets are those of options.h.
  The same day the ways went from eight to four and Manual (the maintainer's call: anything else is an
  order); Holdfast lives on only as what a walk or Halt order of the player's leaves (ST_HELD), and the
  Bodyguard's walking and the parrying stances went with their ways.
- 2026-11-01 (1.4.5): a structural pass after the round of fixes (the code sweep; the pictures only for what
  this round added). sheetview.cpp had grown past 1,600 lines with two things in it: the character sheet's
  pages, and the parse of a saint's help into chips with their ranking against the other saints, which the
  prayer screen, the margins and the tooltips also read (saint_effects_text). The parse moved to
  ui/saintchips.cpp (saintchips.h: the chips, the filter's saint_effect / saint_touches / saint_key_count);
  the page drawing stays. ui/saints.cpp held both the saints' descriptions and what is at work on the party
  (the event table read into effects_now): the latter moved to ui/effects.cpp beside effects.h, with the
  roundels' names (EFFECT_NAME, read by check_docs for {*Fire}) that rich text had kept. One home for the
  campfire picture (ui/marks.cpp: the margin's C, the overland view's stop at 20:00, the icon sheet; it was
  margins.cpp's with a wrapper). The struck shield is one function (ui_key_icon_struck) for the relic's row,
  the rich text's {~Virtue} and ui_shield_value (UI_KEY_STRUCK), not three hand-drawn strokes. A load from a
  native screen had left ns_unwinding set (the stack crept 4 bytes a battle tick until the game crashed):
  cleared in hook_run, the one place every hook passes, rather than in the hooks that read it. Not done: the
  play-through as each kind of reader and the sweeps of words and switches (the changelog and the new
  switch's texts were read; COPY.md regenerated, not read through); fixes.c (2,755 lines) and encounters.c
  (1,909) are past the mark but each is one subject (the fixes' table, the encounters' engine).
- 2026-11-01 (evening, 1.4.5): standing order 4 over the loads in a battle, at his word ("several patchwork
  solutions with saves corrupting when loaded from battle"). Three roots found and the patches over them taken
  out. (1) The mods read item definitions from the game's copy in memory, which a battle or a dungeon overwrites:
  Ingredients of one quality took nearly everything for an ingredient on such a load and set it to 25 (since
  1.4), and relics_frame had been mending the relics, which hid it. The mods now read DARKLAND.LST through
  gamedata (gm_itemdef_u8/u16); the gear in use is mended once from the members' own records of it. (2) The
  riddle-load detour (a load inside the card screen handed to the game's own, 43d6aa2) went: its cause, a stale
  ns_unwinding, is cleared in hook_run; 12 riddle loads, no crash. (3) Seven ways of noticing a load (three
  hooks on the save's reading, one only while quick saves were on; four modules polling a time set whenever
  any .SAV was opened, the Load list too) became one: saves_on_loaded. Left as they are: the events read from
  events.tmp in a battle (that is where the game keeps them then), pack.c keeping both loads around the game's
  own equip_weight (the game's code reads its definitions there), and the repair of old battle saves' trigger
  routines (data, not a path).
- 2026-11-01 (night, after 1.4.5): a structural pass and a look round after the single-file fix and the words
  sweep. One rule now says who moves a member: a follower in single file is the game's until real combat breaks
  the line (ways.c in_line; the game gives each follower the one before it at every step of the leader). The
  ways' automation and the step aside for a line (orders_may_move, read by battlerules.c) both ask it, so no
  module of ours fights the game's single-file step over a member (his Moresnet log: "member 1's foe 0 fell"
  every moment of a fight). The camp's after-camp risk is said once in lodgeview.cpp. Words: docs/COPY.md read
  through by a second reader for lines that leave a rule vague or state it wrongly; two wrong (a heavy load's
  blows, armor's effect on a blow) and some twenty vague ones fixed, each checked against the code or the
  decompilation. Left: the "Many / Some / A few encounters" tiers (not checked), "about a day" for the saints'
  help (durations not checked), and the cathedral donation's Virtue (margins.cpp says "maybe", citynav.cpp
  does not; the donation's code, s056 1abb:0212, is not decompiled). Then traced and fixed: a battle save made
  while the party walked loads with the members marked as walking (cbt_moving 1) and no order (his dksaveb4):
  the battle's start clears the orders but not that flag, and cbt_step walks only on an order with the walk
  bit, so they stood for good, while the ways left them alone as members walking. The ways' tick now sets
  such a member standing (order 0 and walking can only be this).
- 2026-11-02 (night, 1.4.6-RC5): the dwarf king's door again (issue 9). The ladder notice of RC4 answered the
  wrong question; the player's trouble was the card behind the riddle. Traced at run time: the door's trigger
  (type 8, s014 19cd:24a8), its riddle card, the card's -3 and the wall removal on the battle's restart (s023
  1ef8:0b6e) all run as in the original. The cause is the riddle table (s009 182c:1cb4), sampled by calling it:
  battlefield 0x0b (the red-hat king's mine, $MineS00) section 8, the king's door, draws $MineT24..T35, and
  T24/T25 lead to $MineB02, the "go away" card section 10 always brings. Fix kingdoor hooks B02 (the card
  before it, prev_screen) rather than the table, so mines already built, his save among them, are mended too.
  Also $MineB03 never set $NamedOneName ("It is  again"): cardslips. Not checked: whether a random mine level
  ever holds the disabled enemy group that $MineB00's "his minions charge forward" (-4) wakes; in his mine
  there was none (one group, not disabled), so that card would bring no fight there.
- 2026-11-03 (1.4.6-RC6): every prayer by a relic's bearer crashed, since 1.4.5 (his dksavebe: "interp: opcode
  df"). relics.c's saint hook wrote the favor back by SP after hook_call_original, when the call's return had
  moved SP up 4: the word it hit was saint_finish's saved BP, so BP came back as the favor and saint_pray
  returned through a frame that was not there. Why nothing caught it: release_check opened the prayer screen and backed out; the relic
  tests asked a saint's chance from our own code (ns_call_ptr), where the stray word fell on scratch space; no
  check ever prayed. Now: release_check prays (a relic's bearer, Pray now); tools/check_hooks.py, run by
  release_check, lists any argument read or written by SP after the original (it finds the old line and no
  other); hook_run logs a hook point that leaves SP below its entry or BP changed ("left the stack wrong",
  release_check fails on it; the old relic code sets it off at the prayer screen's hook). Also: Pray now
  passes -1, the leader to the game's routines, and sainttexts took -1 for nobody and gave the game's help.
- 2026-11-04 (1.4.6-RC7): a structural check of this batch, asked for: are these fixes, or quirks patched one by
  one? Fixed where they arise: (a) auto-travel that stalled near Graz, or kept turning into Graz (his log: "last
  step 1000", an encounter at every step). A night in camp's encounter made the next step's encounter certain
  (the weight hook in travel.c). It then waited for a card it was allowed to replace, which a unique card, any card not in the
  list, or a robber knight's keep's (step result 2000, s030 1462:0c44) never was, so every step stayed
  certain. (Corrected the next day: 2000 comes only near the keeps, the one kind of place with a second card.) Now
  the step made certain is the encounter's own, and the flag goes with it. (b) Go to: the place table held
  guesses (barracks, prison, town square, bowyers). Every hub card's option flags were read line by line, and
  each rule now names its card; the Go to sweep has no failures. (c) Rested: one rule in the model (can[0],
  lodgeview.c), which the view and the set-to-work after each day both read; the view had made the test
  twice. (d) Twin places: one test (gamedata loc_is_twin) for both maps. (e) Item 0, the Hand Axe: two
  "id > 0" tests (the What came of it snapshot, the room check when buying) took it for no item; a sweep found
  no other. Withdrawn: a replan around the next tile after 40 steps without getting nearer, drafted for the
  Graz stall before (a) was found. The stall came from the encounters, not the map, so a second guess at it is
  not kept. Patches, said so: "changed from 99 to 99" is dropped at the side message hook (the game's practice
  routine reports a change of nothing). The Game menu is put back if found overwritten. That one's cause is
  not known: one report (a title "MeetA01", greyed). The game writes that entry only once, at start (s000
  0e76:083e, no other write in the disassembly). The log now names the card on screen when it happens. Left
  as it is, the largest piece of patchwork: Go to's table is still a hand-kept copy of what the hub cards
  offer. The end state is to read the cards' own option conditions at run time (citynav already parses their
  lines).
- 2026-11-04 (1.4.6-RC8): a deep sweep, asked for ("make every lasting fix you can think of"), by three audits
  read against the code: state that can stick, writes outside what they mean to touch, and copies of the game's
  own data. Found and fixed at the cause: the "MeetA01" menu was text_expand (s043 150b:06fe) filling a card's
  text into msg_buf (DS:9085, 900 bytes) with no limit, and three of 1.4.5's encounters came out at 906 and more,
  over the menu bar's variables and table (the Game title pointer took "ma" of "...and march.", 0x616d, which is
  "$MeetA01"+1). Now: text_expand is hooked (cardview.c expand_guard_hook: the bytes after msg_buf kept and put
  back, the text cut at 899); encounters.c check() rejects a block that can come out longer (each $variable at
  its longest), and datamods.c cuts a mod's block past 899 (the raw copy on text_expand's stack, 900 bytes, has
  the return address after it); five blocks shortened; the menu guard of RC7 removed. Prayers: saint_screen asks
  who a prayer is for only when made from the sheet; a card's (saint_pray_party) and P passed -1, the leader to
  the game's routines: fix prayfor (asked as the sheet's; -1 at saint_finish becomes the one who prays). State:
  losses.c's battle counter went to -1 after a load in a battle (after_load set 0, the unwinding then took one):
  no more "Lost for good" notices; travel.c kept the old game's camp at 20:00 and pending trip across a load;
  saves.c kept a card menu's switches across visits. Copies: the saints' and potions' "how many encounters ask"
  counted only the decompiled card scripts (five read in the disassembly now added) and none of the mods'
  encounters (now counted from what is loaded: encounters_asks_saint/item); relics.c's table of each saint's
  least favor gave way to asking the saint's function; Hardarmor's armour by grade is one function; the shops'
  wares in the margin and the finder follow the barter masks; travel.c reads DARKLAND.MAP through the data mods,
  and the UI's game data is read again when the mods change. Go to's list now follows what the hub cards on show
  offer (and a walk that finds no way), logged as "goto: ...". Also: an encounter's `injure -1` is the leader
  (the game's routine takes -2 but not -1), and its robberies keep item counts in bounds whatever the itemcount
  fix's switch. Left: journal.cpp's stock forecast caps a city's size at 8 where the game reads a ninth row (only
  a modded city of size 10+); the encounters' enemy names are ENM indexes as the game ships them (a mod that
  reorders ENM would mislead); cardhints_data.h's 283 reasons, offices.c's rules and the place-type labels are
  not checked against the cards.
- 2026-11-04 (1.4.6-RC10): asked how to find bugs before a release rather than after it. Two tools, both kept:
  mods/checks.c watches each frame for what must never happen (the menu bar's title pointers changed, a member's
  item count out of range, money below 0, the place or party size out of range, the date going back without a
  load, 285 of the 300 notes in use, 12 map steps in a row each an encounter) and writes one
  "check:" line with the card, screen, place, tile and date; the first of a session also makes a report zip and
  tells the player (ui_check_noticed). A value must stay out of range two seconds (the opening has no party: the
  first soak run caught the check firing at every start; a load passes through such frames; the second, a
  quickload taken for the date going back, twice: no date is looked at while a load of ours is asked for or
  under way, saves_load_under_way, nor for five seconds after any load; DL_CHECKS_LOG traces the dates), and the
  party's checks wait for a party seen since the title (the Windows build sat over two seconds on the start's
  cards with none). A review by a second reader (2026-11-04) took out the check of the party on water: a saint's
  water-as-land help that runs out mid-crossing leaves the party walking on water as the original allows (s030
  1462:1300 stops only a step from land), so it would have reported a fault in plain play; a date read back is
  forgotten once the date is where it was; the florins (unsigned past 32767, as the rest of the code reads them)
  are no longer checked for below 0. tools/soaktest.py plays for N minutes through nctl (cards' options at
  random, Go to and leaving towns, journeys to near towns, F12, the sheet, fights on auto) and fails on any
  "check:", FATAL, hook or encounter error, or a state that does not change for 15 actions; it replaces
  monkey.py (random cards, journeys and battles, no checks) and can start from a player's save (--load). release_check.py
  now fails on "check:", "cards: ... cut at" and "goto: ... not found". Found at the cause: the river trap
  near Graz was travel.c's weight hook making every step an encounter while one was due, on a river too, where
  s030's card draw has none and the step goes nowhere; now only on land, one try per tile (the final soak run
  lost four steps by a bridge near a town, where the draw found no card: the next step from that tile is now the
  game's own), given up after six tries. The pointer's hot spot was learnt from where the game's sprite stood while the mouse stood still;
  inside our own screens the game's sprite stands still too, so the map's (5, 0) stayed: now read from the
  game's own variables (DS:6ef6/6ef8), the gauntlet always (0, 0) (its fingertip is its top left pixel). The
  map's (5, 0) is the original's own slip: s030 takes a click at the mouse itself (1462:02e6, mouse less 0x41,
  0x15, the tiles' origin), so there too the fingertip stood 5 pixels left of the click.
  The last soak run found a real one: a save made during a card's text (the soak's quicksave, during a failed
  climb at a city's wall at night, $CityW01, whose loop shows its menu again) left the menu with no text and no
  options. save_game (s037 1462:2a68, its "Saving Game" box) wrote over the start of the card's .msg text (DS:a895;
  udsdump before and after) and msg_buf: saves.c's save_hook keeps both across every save (reproduced, then 3 of
  3 such menus whole).
  Content: 33 new encounters asked for saints, potions or things no original card asks for; replaced by ones
  the original cards ask for. The original's counts are now in one place (saintinfo.c: saint_info_cards_game,
  potion_cards_game, item_unasked_game; nsdraw.cpp's copy of the potions' table went), and enccheck with ENC_DIR
  fails on an encounter that asks for one of those (release_check builds and runs it on port/content).
- 2026-11-05 (1.4.6-RC13): his RC12 reports. Looked back at the hardest of them for patchwork. The freeze in a
  mine came from the port, not the game: dl_poll hands over the ticks due after a slow frame together, and
  wait_ticks (root 0000:4402, show_message's pause) waits for its tick byte to equal start + n, so it could step
  past and wait 256 more. The answer is at the cause, in the one place the ticks are emulated (runtime/bios.c,
  installed before the mods): the same wait counting the ticks gone by. Only two callers use that byte so; no
  other loop compares a clock by equality (the battle's compare with >=). The relic and the minerals (fixes
  dwarfrelic, carditems) are each one hook on the game's own path (the loot's distribution, the item records'
  adding), not a patch on the card; minechests clears pictures at level_switch, where the floor is built. The
  stall log (no key or click asked for 2.5 s while frames are drawn) found the freeze in the Wine check of the RC
  and stays as a diagnostic, with the native stack (module + offset on Windows; the runtime's resolve with an
  unstripped link of the same build). The alchemy list's marks moved from formula to potion in keep.c, one
  storage, old marks read once. The game's messages over our screens: one layout function for the three kinds.
  Not checked: a translated game's message headings (the outcome and the shield go by the English words).
- 2026-11-06 (1.4.6-RC16): his RC14 report, a loss notice in camp under the clock and F12 under both. The cause
  was layering, not the notice: the clock (and the game's own message) are drawn on ImGui's foreground list, over
  every window, and the notice took the focus each frame, over F12. One rule now, in ui.cpp's frame, in the order
  ui_event hands out events: what is drawn over everything gives way to any window over the game (windows_up),
  and the notice waits behind the windows that take events before it (F12, the welcome, the tips), its OK guard
  starting again when it shows. The cards' saints: the 99% favor is reckoned by the Saints page's own function
  (sheetview.c pray_member, now shared), not a copy. The new toolchain (mingw-w64 g++ 13) no longer brings
  math.h through other headers: the eight files that use its functions include it.
  Ammunition (his wish): the three switches (endless ammunition, endless thrown weapons, thrown weapons given
  back) became one choice, assist.endless (1 endless, 2 given back, 0 the original's), its values the old
  group's so saved settings keep their meaning; ammo.c owns the mode and sets the modules' variables, loot.c's
  give-back takes arrows and quarrels as it took thrown weapons. A choice can now be a list with words for each
  (OF_LIST, choice_desc). Found while at it: a group switch named its group by its place in BUNDLES, so taking
  one out shifted the next (map.overland_marks); options_sync_bundles now says at start when one does not match.
  Also the covers drawn over everything (loading, Go to) give way to a window over the game, as the clock does:
  the first battle's notice, come up during a load, stood under "Loading...".
- 2026-11-06 (1.4.6, rebuilt): he still could not keep a world map section open, on RC15 and 1.4.6. I had
  called it fixed from a test where the right panel was already wide. With his settings (both sections folded,
  the panel narrow) it reproduced at once: the press on the arrow opened the section, the panel, anchored at its
  right edge, grew to the left, the title came under the press position, and the release closed it again. The
  RC15 patch tested the press position against the node where it was now; the fix keeps the decision itself,
  made when the button went down (third_party/imgui TreeNodeBehavior, dlp_arrow_press_id), so a node that
  moves under the pointer cannot turn a press on the arrow into a click on the label. The panel also keeps its
  open width when folded, so it no longer jumps. Lesson: test a report from the player's own settings file.
- 2026-11-07 (1.4.6, rebuilt again): three of his reports. (1) Leaving Helmond put the party right of it.
  overland_open's west clamp set the marker to 0x82 - (x - 8), a pixel per tile instead of 16 (nine places
  with x < 8). Fixing that alone left the party a tile east of the icon: the view draws column 0 as column 1
  ("if (col == 0) col = 1") while the marker's tile is counted from the view's column, so any view at column 0
  (that placement, or a scroll west, which clamps at 0) drew the party a tile east of where the game had it.
  Not patched at the one placement: the view's first column is made 1 (and its row, which overland_open also
  lets be 0) in a hook on the view's drawing, the tile kept (fix westedge, 1462:1c76 and 1462:20d2). Checked
  against leaving Nesse (x 37): the party on the icon in both. (2) The battle flicker, open since RC13: about a
  third of a running battle's pictures were taken during combat_draw, which takes the pointer off and redraws
  the people on the screen itself (dumped: the pointer gone in four of ten). plat_present now keeps the last
  whole picture while combat_draw runs, as it already did after a view scroll (one mechanism, battlescroll.c's
  draw hook). (3) The alchemy seals' digits read as counts: they carry the flasks' grade mark instead (chevrons,
  or dots, by display.potion_grade). His "world map tabs still shut" report was made on build 1200 (the first
  1.4.6 zip), before the fix in 1201; 1201 tested under Wine at his 3840x2160, 150% and his ini: both sections
  stay open from the arrow and from the title.
- 2026-11-07 (1.4.6, build 1203 reports): the hold during combat_draw made battles stutter, "worse than the
  flicker", fast-forwarded or not: it kept the window's last picture while a frame was drawn, so a frame
  finished and drawn over again before a picture fell outside combat_draw was never shown. The elegant form of
  the same idea: keep the last whole picture (when combat_draw finishes, and at every picture taken outside it
  or the view-moved hold) and show that while the battle's picture is not whole. One mechanism now serves both
  holds (platform_sdl.c, keep_whole); measured in a test battle: 15 frames drawn and 15 shown every 2 s, with
  half the pictures taken mid-draw. And the encounters' rep_change (new since 1.4.5) clamped to 99 a Nurnberg
  at 168 after a good deed (-69): cards ($Polit00) and events take the opinion past 99 and the game's own
  chance is then 0; a change now moves toward its limit only, never back. His builds: his logs show build
  1200 until 1203 was installed (1201 never ran), likely two downloads of the same file name: the zips he gets
  now carry their build number in the name.
- 2026-11-09 (next version): three of his asks. (1) What came of it missed a card's gifts after a battle (the
  caravan master's gun, $Rescu00): the note was dropped every frame away from the cards and taken again only at
  the next text, after the card had already given. Not one more special case: the note is now dropped once,
  as the cards are left, and taken again when a screen the card opened closes (run_battle with its loot,
  loot_distribute, barter_screen: encounters.c gscreen_hook), before the card goes on; tested on $Rescu00
  (card 0x105): "+1 Brass Handgun" under the thanks. (2) The level map kept what was explored per game, so an
  earlier save showed the later exploration: saves.c now tells its modules of every save written
  (saves_on_saved, beside saves_on_loaded), and levelmap.cpp keeps its records with each save
  (levels/darklands_levels_<seed>_<save>.bin) and takes them up on load. (3) The manor rumor ($CityN00 block
  2), reconstructed content, off: shown one time in three on "ask about affairs elsewhere", $Direction to the
  place with icon 25, until the journal records a visit. The chronicle's "empty paragraph" (an open item since
  1.4.6) was looked for and not found: entries with list and battle options read cleanly; dropped.
- 2026-11-09 (next version, style pass): his standing order on consistency is a style director's review, not
  "shields wherever an attribute is named". Looked over the ten README screens for one visual language.
  (1) The level map was the one full screen in grey on black with light text: its title, floors and legend
  now sit on the world map's parchment panels in a gilt frame (levelmap.cpp lm_panel), dark ink, the floor
  shown outlined in red as a chosen tab. (2) The battle log wrote a wound as "(-9 End, -3 Str)" beside the
  margins' shields: now the Endurance and Strength shields with the loss ("({Endurance:-9}, {Strength:-3})");
  rich text no longer breaks a line inside a run with no space in it, so "({Endurance:-9}," stays whole.
  (3) Quest titles used "@" ("Robber knight @ Goslar", "Sabbat @ Pagan Altar"): plain words, and the places
  the game names only by their kind read as such everywhere ("the pagan altar near Hall"; one helper,
  gamedata.cpp loc_common_noun, for the quest log, the margin's travel line ("Traveling, near a cave", not
  "near Cave") and the world map). The map tooltip's "(by 9 July 1405)" bracket became a sentence. (4) The
  age wore an attribute's sable shield: the shields are the attributes', skills' and the party's standing
  (which cards test); the years are none of these, so the hourglass stands alone. Two README shots retaken.
  Also answered: the town-square "easter egg" is the spider warehouse poster ($DPost00): $Speci00's first job
  shows where (city +0x56 + party seed) is a multiple of 20 (signed), until taken (event 0x65); in his save
  (seed 99): Flensburg, Nymwegen, Marienburg, Nordlingen, Salzburg; checked in game (Gorlitz greyed; with the
  seed set so it qualified, the line led to the poster). $Offic00 holds ten notices no code shows (robber
  bounties, the Fuggers' mining job): candidates for reconstructed content if he wants them. Go to evaluating
  the cards' own conditions: declined (card start-ups load text, roll dice and draw; running them unseen needs
  a full save and restore around each), the hand-kept rules being checked and self-correcting.
- 2026-11-09 (next version, his answer on the style pass): the shields are for what a member or the party has
  for a long while (attributes, skills, age, fame, a town's opinion), the roundels for what passes (blessings,
  potions): the age keeps its sable shield (the hourglass-alone change undone), and Party Info (F6), the one
  screen naming Fame and Reputation without them, now shows their shields as the margins do.
- 2026-11-09 (next version, loose ends of the day): (1) the "click on Ursula's box moved the view to a black
  corner" seen once while taking screenshots was the double-click (the view goes to the member), in a small
  castle room with nothing around it: reproduced with two clicks, not with one; no bug. The box's tooltip now
  says so ("double-click: and move the view there"). (2) Twice today a generated text behind the code was
  found only by the release check, costing a rebuild and a second 20-minute check each time: a RELEASE build
  now runs check_docs.py first and stops (port/Makefile docs_ok). (3) More screens looked over for the style
  pass (the keys window, the Darklands+ window's journal, the sheet's Items, Saints and Formulae): of a piece;
  the formulae's own descriptions name attributes in words, being the game's text (left). (4) The new quest
  titles read in a game: "The mine at Zbraslavice", "Sabbat at Achlum". Not looked into: the party seed is the
  BIOS tick count since the program started (99 in his game), where the original's was the time of day: games
  still differ, within a narrower range.
- 2026-11-09 (next version, his report 203017, build 1204): members told to parry "often change to
  vulnerable". kept_kind (ways.c) was meant to hold the kind, but (1) it was noted only on a click at a foe,
  not for a kind chosen from the Orders menu or the buttons against the foe already fought (the usual way to
  say Parry); (2) keep_kinds put it back only over a Std Attack, and to_melee opened on the next foe with
  what the armour asked (Vulnerable); (3) a fallen foe (order 0) forgot it. One rule now: Parry and Berserk are
  the member's stance, noted where every order passes (leave_hook), used by to_melee, kept over a fallen foe,
  and a member on Parry takes only a foe beside it. Tested in his castle fight: 0x22 noted from the P key, no
  new foe sought after the foe fell.
- 2026-11-09 (next version, his answer on Parry): a member standing on Parry with no foe "is too confusing":
  the stance now ends when no foe is beside the member (it fights on in its way). And as he asked, a member
  with two or more foes beside it parries by itself (Improved combat; Parry still strikes, slower, and is
  harder to hit: s014 18bf), back to its attack once fewer have been beside it for 40 clock ticks (foes step
  in and out of reach: without the wait it flapped every tick), not over the player's stance or an order
  given while beset. Tested: Ursula alone against the castle's two: "has 2 foes beside it: parries", once.
- 2026-11-10 (next version, his reports 220630, 221057, 221951, build 1214): (1) "the red door flashes and I
  am back in the dungeon": the door's trigger ends the level with result 7 and runs card 0x16b, $Witcd41
  (found by the overlay trace: s134), whose code loads the picture and returns to the level without showing
  its text 0 (the empty cell); its text 1 (a captured member found) has no code. MicroProse's own slip:
  text 0 is now shown (fix cardslips, celldoor_hook). His "Artifice +4" was the click on a locked door
  before it. (2) "why is Ursula waiting": a Charger behind her friends in a passage, no way to either of two
  foes: ways.c remembered one unreachable foe, so idle_fighters sent her at the other, and back, at every
  tick (hundreds of log lines). Every unreachable foe is remembered now (noway_at[5][13]); with none left to
  go at for want of a way, the box says "no way" (ways_blocked). His "Chargers still shoot": the volley was
  loosed while they were Skirmishers; none after. (3) "4 against 4 and two still parry": auto-parry counted
  foes within reach, which in a press is all of them: it counts foes whose target is the member.
- 2026-11-11 (next version, his asks and reports 102254, 110134 and the Templars' fortress, build 1216):
  (1) Potions: SV_SHARE (sheetview.c) gives one at a time (give_one) until every member on their feet has
  the same, the odd ones to the first; the battle tray adds the chosen member's drinkables (sheet_drinkable),
  drunk at a safe point (sheet_battle_drink: use_potion, then item_take_one, as the sheet's Drink); every
  potion's tooltip ends with ns_potion_effect. Shared potions only looked into (his ask: no implementation).
  (2) Effects on fighters: mods/foefx.c reads cbt_mods (DS:a532: the lowered attributes and skills, held, and
  the three ends on the battle's clock) and watches effect_spell (s019 1ef8:0000) for which potion set each
  end; ui/battleoverlay.cpp draws a roundel above the head and lists them in the fighter's info. The throw's
  wind-up (action 11 until DS:d948[m], s014 0e76:1066) shares the reload bar.
  (3) The level map: what the view showed was put down for the floor shown before lv_keep had settled on it,
  so during the stairs chain below a floor got its neighbour's rooms (his "areas beyond the walls"); marking
  now waits for it, and remembered floor is drawn only where the floor has floor art. The chain itself: the
  single-file followers' walks went on onto the stairs they had arrived by (take_exit_hook: a floor change
  within 2 s of the party's, out of a fight, stops the member instead). The ways out (20a5:3326, 10-byte
  records; the "near an exit" bit, s014 19cd:209a) are drawn: the fortress's is the graveyard hall's north
  row and east end, found with uallexits (every floor loaded in turn), tested by leaving it in his save.
  (4) One rule for orders in a pause (orders.c give_now): out of a fight every order runs at once; in a fight
  the first order of a pause says it waits for Space. A click on stairs, a chest or a way out walks there
  (levelwalk.c levelwalk_door_click); a chest or way out then chooses the leader, so its orders are on the
  bar and a trap is not sprung for the player.
  (5) Tooltips: none while the wheel turns or a button is held (BeginTooltipEx); a row of a list gets its
  tooltip at its side (tip_auto); the fighter's info in a top corner of the field; "Can't walk there" said on
  the click (orders.c click_hook), not under the pointer; the learned saint goes with its card.
  (6) Copy: "fighting style", "no path", "no clear shot", "no ammo", "permanently", "continue", "route";
  the load tooltip a line per effect with its numbers (ui_load_lines).
- 2026-11-11 (standing orders, after RC7): (1) Structure: levelwalk.c has one ground_at (the ground point
  under the game's pointer, figures excluded) for the stairs, chests and ways out; the chest lookup no longer
  repeats it. (4) The stairs chain's fix was a timer (a floor change within 2 s of the party's refused):
  the cause is a follower stepping onto stairs on the leader's trail, so the rule is now the game's own
  logic of single file: there only the leader takes the party to another floor; a follower's step onto
  stairs stops them (a group or a fight: each member stays the player's to send). Tested in his save: one
  U, the party on floor 11 together, the three followers' steps stopped. (2) Style: the inn's rows named
  the game's "Regain strength (gain 2)" and "Pray for divine favor (gain 5)" in words beside rows that use
  verbs and shields: now "Regains {Strength} (+2)", "Prays for {Divine Favor} (+5)", as the right page.
  A skill gain in a battle named "Missile Dvc" in words beside the other skills' shields: the game's short
  names (Missile Dvc, Speak Common, Speak Latin, Read & Write) now find their shields. Looked at and left:
  the camp (of a piece with the inn), Settings (its section pictures differ in kind but read at a glance),
  Save and Load (the party named in words: those members are the save's, not the party's, so no heads).
- 2026-11-11 (his asks after RC8): (1) Potions: the per-row Share and its automatic sharing are gone for one
  "Split evenly" on the sheet's Potions heading (SV_SHARE, id -1): every kind (id, quality) in one order,
  each split among the members on their feet, the odd ones to whoever is to hold fewest so far, so a second
  click finds it done and the members' totals come out even too (his save: 41 each). The battle tray is two
  rows, a goblet before what the member drinks, a flying flask before what they throw or use. (2) A chest
  clicked out of a fight: opened on arrival (location action 5, as Open Chest) only when it is neither locked
  nor trapped; otherwise the leader is chosen and the lock or trap named, Pick Lock or Disarm left to the
  player. Its hint names the lock, or the trap of an unlocked chest, once a member has stood at it, as the
  game's own orders there tell (a locked chest's trap stays untold until it is picked); the margin's "To
  explore" help says the same. (3) Found on the way
  (his save, the margin's numbers doubled with a sheet open in a battle): the battle's sheet (s031
  1462:0000, called inventory_leave until now: it is world_load, member_sheet, then events.tmp written and
  the table freed) wrote the cards' events over the battle's first slots and sent the rest to the file, so a
  potion drunk in the fight stopped counting. fixes.c battle_sheet_hook moves the battle's events past the
  loaded slots and takes them aside as the sheet closes (fixes_sheet_closed, from sheetview's member_sheet
  hook), putting them back after the write; effects_now and pot_all use the table alone while it holds both.
  Tested: Deadly Blade drunk, sheet opened and closed, bonus byte 40 throughout, the event still there.
  New test command uchest (a chest's lock and trap set, the floor's chests listed).
- 2026-11-11 (his answers to RC9): (1) Split per potion, not every potion: the heading's Split evenly is gone, a
  Split on each potion row of a member's own (left of Drink, or in Drink's place for one only thrown) splits that
  potion, every quality of it; the odd ones go to whoever is to hold fewest of it so far, then to one who holds
  more of it now (fewer moved). Tested in his DKSAVEDM: Essence of Grace 9/8/8/8, a second click "already split
  evenly"; Black Cloud, two qualities, 2/1/1/1. (2) The tray's marks "more abstract" (the goblet read as Divine
  Favor, the flask as a crushed bug): one family of plain signs, a gold dot for the member and a pale arrow for the
  potion, down into the dot to drink, off on a dashed arc to throw. (3) The sound: no preset touches the device any
  more (options_sound_kept gives back a device the purist presets had switched to "game", at the start and on a
  preset choice); Full Original's description says the MT-32 stays.
- 2026-11-12 (his questions and asks after RC10): (1) Quality: the game reads a thing's quality for weapons,
  armour, shields, potions, a horse (riding checks), a lock pick (lock_pick: Pick Locks + quality - 25) and what
  a card lets the party use (card_items: potions only, every card); goods are asked for only by count (rope,
  lock picks, an instrument). So "Ingredients of one quality" (loot.c is_ingredient) now takes the goods but
  lock picks too, and ns_shows_quality shows a tool's quality only for lock picks. Tested: two flutes 20q and
  35q joined at 25 on a load, lock picks kept 35 and 20. (2) Essence o'Grace's "+278 Endurance": each dose's
  raise_attr_temp is an event (0x48), and with the blessing fix a wounded member's boost only heals;
  effects_now (boosts_in_force) now shows a boost as what stands above the maximum, the last to end keeping it,
  and none where nothing does. Tested: a dose at 20/39 shows nothing, a second at 36 shows +15 (54/39). (3) His
  Jorg at the inn, Regain at 37/37: the game sets a member back to relaxing only in the day's own regain, so a
  Strength filled on a sheet (a potion) waited a day. lodgeview.c rested_to_work(m, 1) runs every round for
  Regain or Pray with nothing left. Reproduced (uww, F2) and fixed. (4) Ingredient tooltips: the three nearest
  sellers (journal_where_sold_list), placed beside the shelf, not over it. (5) Potions an effect a line with
  amounts and how long: thrown durations read from the game's tables (DS:21e6, 2202, 221d: Noxious Aroma 6/9/12 s,
  Eyeburn 8/16/24, Fleadust 25/40/60, Sunburst 4/6/8, Black Cloud's cloud 20/60/180, Stone-tar 240), drunk ones
  measured (24 hours asked, 19 to 21 had). (6) A stock to keep (keep.c "stock item n", a click on the shelf) and
  a notice on reaching a town whose shops (seen or worked out) sell it (journal_stock_check). Test command ustock.
- 2026-11-12 (his "overboard" on the alchemy screen's grade seals): the seal's tooltip repeated the formula's
  name, mystic number, risk and batches, all on the right page after a click, and covered the rows below. Gone;
  the row's second line now tells of the grade pointed at ("enough for 7 batches", "needs Aqua Regia").
- 2026-11-12 (his asks after RC11): (1) The alchemy tablet's chart: at most ten bars, a window of ten that ends at
  the batches set past ten (held still under a drag: V.drag_first), its ends numbered when it is a window.
  Tested: 200 of each ingredient, 50 batches allowed, ten bars. (2) Fix "bestpick" (fixes.c, on GF_lock_pick,
  inside pack.c's fetch): lock_pick reads the first record of item 0x40; for the call it takes the best quality
  the picker (the shared pack: the party) carries. Tested: lock picks 25 then 45, "picks with quality 45, not
  25". (3) The shop's For sale under headings by kind (ware_group), counts, a click folds a group
  (display.shop_folded, every shop); a ware found from the Formulae tab opens its group and is scrolled to.
  (4) foefx_of: one entry a potion (Eyeburn's blinding folded into what it lowers).
- 2026-11-12 (the spare-gear crash, RC10): the timer handler (047c:01ea, recompiled F_root_049aa) reads its
  far call targets from memory as it runs; his log ran the party panel's saved status-letter background
  (gfx_save_rect, D_a6c2 = 480a, as the huge pointer 4000:80a0) as code on the timer's own stack. The cause
  was not reproduced (his save, 25 runs, a 30-minute soak). bios.c now checks those targets at each tick
  (puts back and logs one changed) and holds back an interrupt whose handler still runs; interp.c keeps its
  last 16 entries and far jumps for a crash's log and logs code run outside the program and its drivers.
- 2026-11-13 (structural, his asks after RC12): (1) Where the party is on the map was worked out three ways
  (journal.cpp travel_min_to: only on the travel map; party_where: not on a native screen; quests.cpp
  party_xy: the map or a card) - so the days to a seller were missing in a camp and on the alchemy screen,
  and "nearest first" was in town order there. One answer now, travel.c travel_party_at, for all three.
  (2) A potion's effects were one string per potion with the grades' numbers written "a, b or c" and a
  "numbers by grade" line, shown four ways (the shop and sheet strips through ns_item_use, the inventory
  tooltip, the battle tray). Now data (nsdraw.cpp ns_potion_fx: how used, what it does, rows of three
  numbers, what stays) with one block that draws it (ns_potion_about: the game's words, the how picture -
  the battle tray's, now ns_potion_how_icon - the words, the table a column a grade, the one in hand
  marked; beside the words on a wide panel, under them on a page; fitted to its room by leaving out the
  encounters' line, then what stays, then the game's words, then smaller type), its ImGui twin
  (inventory.cpp ui_potion_about, in the item and formula tooltips) and the one line of text for the
  battle tray (ns_potion_effect_grade, that grade's numbers). The sheet strip's second copy of a potion's
  words went; "some risk" on the sheet's Formulae is "medium risk" as elsewhere. (3) The stock to keep:
  20, 50, 100, 200, none; Shift+click sets every ingredient. Test commands uitemtip, uformtip.
- 2026-11-14 (structural, his asks after RC14): (1) The stock notice fired every time (his log: "Sold in
  Lübeck: ... 14 ingredients") and was replaced at once by the autosave's: overlay.cpp held one notice in a
  160-character buffer. Notices are now a stack (notice_add: up to three, newest at the bottom, the same
  text again moves down), and the stock notice is short (the three shortest of, "and N more", the shops).
  Tested on his save: the notice, then a save's under it. (2) The ingredient tooltip was built twice
  (sheetview, alchemyview) in words; one now, nsdraw.cpp ns_ingredient_tip: the sellers as rows with the
  map's hourglass (worldmap_draw.cpp layer_glyph 4) and journal.cpp journal_where_sold_rows /
  journal_days_short (journal_where_sold_list now built from the rows), the potions as ns_ingredient_row
  draws them. (3) The grade seal of the alchemy list is ns_grade_seal, used by it and by the sheet's
  Formulae list, which is now a potion a row like the alchemy screen's (the old row a formula with "have" and
  "brew" went; its page no longer lists the ingredients the shelf rings). (4) PotionFx rows: the clouds'
  effects one a row with the cloud's life in the words ("hangs 3 seconds" against a row "how long (seconds)"
  6/9/12 read as a contradiction), FxRow::same() writes a number the grades share once across the three
  columns (native table, ImGui table, battle tray text); the attributes' shields apart, not bracketed.
  (5) The alchemy chart: always ten bars, those past the ingredients hollow, a line where they stop.
- 2026-11-14 (his asks after RC15): the potion block in shorthand (nsdraw.cpp ns_potion_fx/potion_table,
  inventory.cpp ui_potion_about): PotionFx.keys, the shields a cloud's rows strike, first; FxRow.time, the
  hourglass ("6s", "19-21 h"); the rows the same at every grade above the grades' pictures, one number,
  those that differ under them. A rich-text token for every weapon skill, {Weapon Skills} (richtext.cpp:
  the Saints page's three fanned shields; check_docs knows it). The stock notice is a box as the learned
  saint's (overlay.cpp draw_stock, ui_stock_popup: the ingredients' pictures by shop, have/want; with its
  card), not a notice. Pull request #13 (woodk123, the tomb's item after every group is beaten) merged as it
  was, then its raw addresses named in the decompilation's header (place_won D_9084, which the castle fix
  uses too; battlefield_kind D_c1f0; activations_seg D_7d3c) and gamevars.h regenerated, the activation
  records' offset in foesleft.h; test_tomb.c passes.
- 2026-11-14 (his ask after RC16): the potion table's number columns sit after the labels (fx_labels_w: the
  labels on one line where w allows) instead of at the far side, and a bracket runs from a cloud's shields
  down to the rows they head (native potion_table; the ImGui twin draws the same with the window's draw list).
- 2026-11-14 (release): 1.4.7 becomes 1.5 at his word: CHANGELOG's section "1.5", written short and by
  importance, the same text as the release notes' "New in 1.5"; the code's comments say 1.5 where they named
  the release (the release candidates keep their 1.4.7-RCn names).
- 2026-11-14 (his ask, before publishing 1.5): a click on a chest no longer walks the party there (levelwalk.c:
  chest_pending, chest_goal and the arrival's Open Chest / leader-chosen went; the click is the game's own move).
  Walking in single file left the member who should open it, by Artifice, far from it. Pointing at a chest still
  names it, locked or trapped once known, and says to open it from the orders of a member beside it.
- 2026-11-15 (his ask: "address the pull requests as you see fit", then "check Darklands Restoration for
  issues you missed and credit"): woodk123's eight pull requests #14 to #21 merged, each by its own merge
  commit; they all added at the same places of fixes.c (FIXES, the HOOKS tail, MORE, fixgroups), resolved
  in order, the HOOKS order matching FIXES (the _Static_assert holds). Their release-notes lines dropped,
  the changelog written as one "Next" section, README count and the generated documents by check_docs.
  Tested: their nine C tests on the merged code (all pass), the save check against the 172 saves we hold
  (none refused), the build without warnings, the full release check. CONTRIBUTING.md now asks for the
  changelog entry instead of release notes and for how a player meets the bug. register_crosscheck.md
  brought to the register's 277 entries (121 new: 36 fixed already, 19 by the pull requests, 54 open, most
  of those the party split across floors). Credit: README, the About screen (and blast's licence there).
- 2026-11-15 (his ask: "fix every bug you can", the register's 54 open): 27 fixed or settled. New switches
  thrownskill, noselection, castsight, doorplan, maprows (plain or game-breaking) and fruitonce, footpadsonce
  (old tricks closed); the rest joined the switch they belong to (knightdead, lordreward, membervalue, alchemy,
  cardslips; politics in politics.c). The pattern kept throughout: a card's slip is put right by wrapping its
  routine and, while it runs, the one call it gets wrong (hook_install/remove around hook_call_original), never
  by copying the card. Five are bugs of screens Darklands+ draws itself (loot, party information, cards, the
  saint list; the map's Alt-Q did not reproduce): marked "not reached" with the display switch that brings the
  original back. Left open, said so in register_crosscheck.md: the party split across a battle's floors (21: the
  battle holds only the floor on screen, a rework rather than a fix), the U key paused with nobody chosen, and
  the mine's gap to its own floor. Then the cell buffer's two (campwatch: the camp's count over the cells held,
  checked equal to the original's at 16 places of the view; maprows: the edge icons left out while drawing).
  Tested in a session: famine, the sneak hall's card (its own texts; the old one read the gate's), the night
  street by day, the fruit's Strength after its text, footpads' resume card after the fight, quicksave there.
- 2026-11-15 (his ask: "work on whatever is still open, easiest to hardest; address pull requests and issues"):
  pull request #23 (woodk123, Nearby work) merged, then made one with Go to: a giver's state in one function
  (citynav_office_line/_tip) for both lists, the list called Quest givers like the other finders, its picture
  the three kinds of quest fanned (the chest alone is the treasure quests'). Issue #22 (Seb-Zee, an HD pointer)
  done inside the existing replacement art (art\GAUNTLET.png, a mod's art too; gamepic_art), closed. Register:
  exitleft (a member gone out by the way out set aside for the end test and the stairs' coming together, not
  counted for the group modes), U with nobody chosen (noselection), three found covered by woodk123's
  stairsreturn, two confirmed harmless. Left: 16 entries of one kind, members and enemies on a floor not shown
  (the battle holds one floor): a design decision (keep a fight on one floor, or hold the floors not shown),
  put to him rather than made.
- 2026-11-16 (his answers and asks, 1.5.1-RC4): the 16 split-floor entries closed by keeping the party on one floor
  (fix.onefloor: levelwalk's following in a fight too, the knocked-out carried through stairsfix_carrying), tested
  in a castle fight: everyone on the new floor, the fight paused until the foes who saw follow. His report of six
  encounters in a row by a pagan altar: the six altars add 99 to the weight (DARKLAND.LOC +0x0a; no other place
  more than 5), tried at each pixel step, and the step with an encounter stays put: fix.altarroad (an encounter
  there spends the altar's weight until reached or left two tiles behind), tested at ten times the rate. Style:
  the mounts drawn as a head in profile, coat by quality, a mule with its ears (the horseshoe stays the Riding
  skill's: one picture, one meaning), their words from the code (quality added in chases and flights); the
  shop's "have" tag red under the stock to keep, as the stock notice; the place card on the world map one flow
  (offers, boats in a row, saints, formulae and teachers side by side) with "Scroll for more" while more is
  below; Share all on the Potions heading beside each row's Split; the member's own sections fold as the pack's.
  New encounters on by default with a note once at the start (worded on what can be checked: Dragon's review
  and the CRPG Addict on repetitive encounters; "most common" not claimed); keepitems spares the ridden mount;
  quests.no_papers (Cards and towns, off as in the game) draws a merchant's errand again while it is papers.
- 2026-11-17 (his report: the menu bar's Game over a way out walked the party there; "approach and fix it
  in a systemic way"): ui/layers.h is the one order for clicks over the picture (panel, view on the game's
  pointer, field), ui_event follows it in one place, panels register their rects (ui_cover, mclick and the
  margins' begin_panel all go to ui_layer_rect), the field's own actions and hints ask ui_pointer_on_field()
  (levelwalk, orders' hint, the battle overlay; ui_pointer_on_cover is gone). The open menu's left clicks
  are held from the game (runtime/bios.c mouse_hold_left; menuview waits for the release): before, the
  click that closed the menu also walked the selected member. Tested in a keep: the closing click leaves
  the party standing, the Game title opens its menu.
- 2026-11-17 (his report: potions drunk in Lugum gone after saving there): what a battle keeps of its own
  events is said once (fixes.c battle_keeps, battle_kept, battle_put_back) and used by the battle's end
  (cards_up_hook) and by a save made in it (battle_save_hook, new): the game's save in a battle reads
  events.tmp over the battle's table and frees the whole table after; the hook lends it events.tmp as it
  will be once the battle ends and keeps the battle's table out of its way.
- 2026-11-18 (his asks: potions listed a row a grade; Roman numerals for grades): a potion at several grades
  is one row through one helper, ui/nsdraw.cpp ns_potion_flasks (at rest one flask and the total, pointed at a
  flask a grade with its count, the one the buttons act on lit; the choice kept by thing and quality, as the
  lists are rebuilt after each action), used by the character sheet and the loot screen; shops keep a row a
  grade (each has its price). How a grade shows is read in one place, ui_potion_grade_look() (marks.h; the
  settings' pictures through it), the numeral drawn by ui/marks.cpp ui_mark_numeral (on the flask's badge,
  bare in a formula's seal). Chevrons now mean only the saints' raises and lowers.
- 2026-11-18 (his design for PR #31, Find teachers): who would learn from a teacher is one rule,
  ui_teach_learners (ui/teachersui.cpp), with the teacher's town icon and the skills' short names
  beside it; the margin's lectern and the world map's Find teachers both read it (margins.cpp keeps
  thin wrappers), so a row is dim in the list exactly when the lectern is dim in the town.
- 2026-11-19 (his review of Find teachers): the words for who would learn are one function too,
  ui_teach_who (teachersui.cpp): heads with their skill now, "for the party" on the skills one member uses
  for all, and when nobody would, the same heads, so the reason shows; the lectern's tooltip and the list
  both call it. A teacher not to be had now carries its reason in a few words (TchSource.why, set beside
  each off = 1 in mods/teachers.c), so the list says why instead of "not now".
- 2026-11-19 (standing order 4, the tomb): three layers had piled up. The original gives a tomb's
  thing whatever the battle's result (s039 1462:1ab0 reads cur_loc), so getting out by the way out
  kept it, the design its cards describe (an escape); only its victory (cur_loc already -1) and
  defeat were wrong. Our tomb fix gave it only for a victory, which took the escape away; #13 made
  beating every group a victory to get it back; #29 would have warned at the way out. Now
  tomb_give_hook gives for a victory or an escape (results 0 to 2) and nothing for a defeat; #13's
  victory stays (the fight ends once every skeleton is down); #29 closed. port/tools/test_tomb.c
  checks each result.
- 2026-11-19 (his rule: "always run performance checks"; investigate anything near a limit):
  tools/perfcheck.py now runs as release_check.py's last step, on its own copy of the game folder.
  It measures each screen while it is up (each step ends with its screen open, the next one closes
  it) after 3 s to settle, and it covers the loot screen too. A worst over 8 ms in one window only
  is "once", not a failure; more than 60,000 vertices is "watch". Looked into: the Saints page's
  8 to 11 ms comes in the frames right after it opens, then at most 2.3 ms over 30 s (a stack
  sample of those seconds found the game waiting on the screen); the alchemy screen and the world
  map draw about 62,000 vertices, the alchemy screen's 36 item icons 12,000 of them and the crisp
  text another 12,000. Drawing the icons once into pictures (as the plaited band is) would save
  about 11,000; not worth it while a graphics card draws these with ease and the build stays under
  2 ms.
- 2026-11-19 (his design for PR #34, the weapon's fit; standing order 2): the combat numbers come from one
  place, mods/gamemodel.h: a weapon's line of the tables (gm_weapon), the rating weapon_ready gives, the
  defence, the to-hit chance, the damage through a tier, the Strength term and the time per blow, and in a
  battle the same from the combatant records (gm_cb_*: what weapon_ready set, or for a member or foe with a
  missile weapon readied, what it would set with their melee weapon, read from +0x57 bit 7). The tooltips,
  the character sheet's line, the item line under every list and the battle's foe info all call them. One
  picture a property, ui/combaticons.h (two hands, damage, penetration, the hourglass for time, hit, dodge
  a blow or a shot, the six armor tiers as tunics), also in the potions' and saints' roundels (damage had
  shared the sword with a weapon's quality) and on the icon sheet (uiconstyles:2). The gear tooltip is one
  function, ns_gear_tip (ui/nsdraw.cpp), for the sheet, the shops and the loot screen. The pen-flourish
  frame's inner curl is kept in the corner (it reached into a tooltip's first and last lines).
- 2026-11-19 (his review of PR #36, battle scenery art: "consistent with the other methods"): the
  scenery goes through the one replacement-art path (mods/hdart.c), not a second one. After
  battle_tiles_load and furniture_load each scenery segment is named as a battle frame
  (SET_KIND_RR_V; where the loader keeps them, from woodk123's reading in #36), so gfx_draw_sprite's
  frames, the names under the mouse, the export and Look again take them as they take the fighters.
  What was missing for it: the battle view reaches the screen through gfx_show_page (MGRAPHIC slot 31,
  a whole page copied), which now carries the page's pictures to the screen as gfx_copy_rect carries
  a rectangle's; names up to 23 letters (ART_NAME); up to 1024 replaced pieces drawn a frame. Not taken
  from #36: its own drawing at the game's pixels, the town-only names, F8 and Ctrl+Shift+R (the art
  switch and Look again are the tools for every picture). Measured: a wilderness battle with its floors,
  trees and rocks replaced prepares its frame in about 1.1 ms (0.13 ms with no art).
- 2026-11-21 (his ask after RC15: "pair up each UI element with every other UI element", the potion tray's
  tooltip not shown and a field hint over the tray): every element over the picture paired with every
  other, and one order made for both the pointer and the eye (ui/layers.h). Found: (1) the cause of his
  screen - a window that closes on the press (the level map's click, the world map's double-click, the
  Honor Roll) left the release to the game, so ImGui's button stayed down and the SDL backend, which reads
  the mouse by itself only while no button is down, kept ImGui's pointer where the click was: the panels
  then hovered nothing and the field's hints followed the game's pointer over them (seen with his save:
  ImGui's pointer stood at the level map's click after the mouse moved). ui_event now hands ImGui every
  release of a button or key it saw pressed, and every move of the mouse, whoever takes the event. (2) The
  order on screen was not the order for the pointer: the panels' and the field's windows stood in the order
  ImGui first made them in a session (none is ever focused), the party's boxes were on the background list
  under all of them, and a dozen things were on ImGui's foreground list, over the tooltips (the notices,
  the foe arrow, the route, the camp button, the money that rises in the margin, the battle log's lines,
  the picture names, the town walk's and the hours' covers). Now each is drawn in its tier (ui_tier,
  ui_tier_list: the covers, the field's marks and boxes, the panels, the shade under a window, the
  notices), and ui_tiers_stack puts the tiers' windows under every other window in that order, the notices
  over the windows and under the tooltips. (3) Under a window that takes every event the panels, the
  pages and the field still tested the pointer (the strip's and the tray's buttons by its bare position,
  the pages by the game's): ui_draw hides ImGui's pointer from everything drawn under such a window, and
  NsDraw, the party's boxes and the menu bar take no pointer then (ui_modal_up, also for the game's own
  message over a page). (4) The field's hint keeps clear of the panels (ui_layer_clear), the other sides
  tried in turn; the party's boxes a click picks are panels in the layers too. (5) The open menu bar was on
  the background list, under the panels (the tray covered the corner of the Orders menu and took a click
  there; a click on a margin acted and left the menu open): it is now UI_MENU, drawn over the panels
  (UI_TIER_MENU), taking every click (over the picture through the game's pointer, beside it by closing the
  menu), with nothing under it seeing the pointer. (6) The game's hand is drawn last, over the game's own
  message. The matrix of every pair is published for him (the artifact "UI Layer Audit").
- 2026-11-22 (his report: "permanent perception at 60 should NOT happen", with his 273 saves): found in
  them where it began. dksaveep (Lugum dungeon, 11-17 09:34) has Jorg's Perception 33 of 33; dksaveeq, six
  minutes later at the same hour of the game, 60 of 33 with no boost record. His log of that session
  (build 1281, 1.5.1-RC6) has "Jorg drinks the Truesight" in the dungeon, then "Saved: Lugum 24 Dec
  (dksaveeq.sav)" there: the game's own save in a battle (s037 1462:2ace) read events.tmp over the
  battle's events and freed them, the boost's record lost and the attribute left raised. That save was
  the cause of his "potions drunk in Lugum gone" report the same morning, and battle_save_hook (RC7, build
  1289) keeps the battle's events through it; checked again on build 1319: a Truesight (+23) drunk in
  dksaveep and saved there is in the save with its end (25 Dec 18:00) and comes back with it. Not one of
  his other saves has an attribute above its maximum by more than its running boosts. What was missing
  was the mend for saves already spoilt: boost_mend (battlesaints) takes off whatever stands above the
  maximum and the running boosts (kind 0x48, in the table and, in a battle, in events.tmp) once a game is
  loaded and after every battle, with a notice (his dksavees: back to 33; a running Truesight saved and
  loaded in a battle left alone).
- 2026-11-22 (his "why wasn't this bug caught before?"): the same fault, the game swapping its event table
  out (events.tmp) while a battle's events were in it, has three ways in: the end of a battle (battlesaints,
  1.2), a sheet in a battle (1.4.7-RC9, his report) and a save in a battle (1.5.1-RC7, his report), each fixed
  when he met it, not looked for together. Looked for now in the generated code: the callers of world_load
  (s043 1462:0100), cards_shutdown (1462:054e) and cards_startup (1462:078e) are run_battle (s029 0e76:2278),
  s029 0e76:255a (cards_shutdown alone; the cards come up through cards_startup, hooked), battle_sheet (s031
  1462:0000), s037 1c7d:09c6 (cards_startup alone, in the loader's segment) and one card (s133 1838:186c),
  whose calls between the two (101 functions reached) never reach event_create (s029
  0e76:2c4e), use_potion or the attribute boost: nothing can be made there to lose. The game's save writer
  reads the file itself (battle_save_hook). Against a fourth: CK_BOOST (mods/checks.h, watched beside the mend)
  logs an attribute above its maximum and its running boosts outside a battle, and release_check drinks a
  Truesight in its battle, saves and loads there, and fails on the mend's line. And the level map's doors
  (his screenshot): drawn when the centre of their record was in seen[], which takes every cell of the
  view's diamond, behind walls too; now, as the stairs, ways out and chests, only over floor known (rect_seen).
- 2026-11-22 (his report: Dietrich overloaded at 198 after drinking his Ironarm): equip_weight (s031 18e7:140a)
  sums the definitions' weights of what is equipped; item_unequip calls it when an item runs out, and in a battle
  the definitions (far 2e38:0890) are not in memory unless a sheet has loaded the cards' world. The battle's potion
  tray (sheetview.c battle_drink_now: use_potion then item_take_one, no sheet) drank his last Ironarm there:
  reproduced with his DKSAVEEY (test command udrink, the tray's own path): Dietrich's load 61 became 37 (198 in his
  game: whatever lay there). Fix battleload: in a battle its result is put right from the game's item list
  (DARKLAND.LST through gm_itemdef_weight; the equipped record found as s031 18e7:14b6 finds it: the kind at +2,
  that quality first): 61 and 58 kept, the same as before the drink; and a game saved with a wrong load is put right
  once loaded. pack.c's own keeping of both loads around give_one (the same fault, found in RC9) stays for when
  the switch is off.
- 2026-11-22 (his "wasn't part of my standing orders explicitly for you to stop relying on patchwork fixes?"): it
  was, and it had been kept as a look back at old fixes between tasks, not as the bar for each new one: the load
  of 198 was the cause found in RC9 at pack.c's hand-over, written in its comment and guarded there alone, and
  that morning's fix battleload put equip_weight's answer right without touching why it was wrong. The rule is now
  in CONTRIBUTING.md ("When a fix is finished": the cause in the game's terms, every way to it looked for and the
  fix at the cause, a check against its return, all of it here). Applied to the load. The cause: the game reads
  DARKLAND.LST into a DOS block and keeps 200 far pointers into it at 2e38:0890 (world_load s043 1462:0100, the
  loot's s040 1462:0000, the start's s001); mem_free (root 05ba:00c2) frees the block at cards_shutdown, at the end
  of the battle's sheet (s031 1462:0000) and of the loot, and the pointers stay, into memory the battle takes. The
  ways in: every game function that reads the table (53 functions touch it, in s001, s029-s043 and the cards; none in the battle's
  s014-s028, which call only the sheet and the loot, and those read the list again) when called in a battle - the
  original never does, Darklands+ does: the potion tray (use_potion, item_take_one), the pack's give_one, any
  ns_call to come. So it was Darklands+'s fault, not the original's: the switch battleload is gone (94 fixes; its
  text had also claimed that throwing the last of a thing spoilt the load, which no code does). The fix at the
  cause (mods/nscreen.c mem_free_hook): when the game frees the block the pointers lead into, the records are
  kept at ITEMLIST_SEG (F400, memory the game never uses) and the pointers moved there until the game reads the
  list again. Seen with the test command udefs (the 200 records against DARKLAND.LST): 67 of 200 in his DKSAVEEY
  before; 200 of 200 after, through a load into a battle, the sheet opened and closed there (the game's own block
  while it is open), a load out of the battle and back; two members' last Ironarm from the tray
  leave their loads at 61 and 58 with no guard. Taken out: equip_weight's hook, pack.c's keeping of both loads. Kept: the mend
  of a load saved wrong (fixes.c load_mend, from gamemodel.c gm_member_equip_load). Against its return: CK_ITEMLIST
  (the table's records are DARKLAND.LST's, in a battle too) and CK_LOAD (each load is what is equipped) in
  mods/checks.c, and release_check runs udefs in its battle. And his Jorg's +4 Thrown Weapons at the stairs: the
  game's own, not a fault: a potion thrown sets the member's practice bit 4 (s014 0e76:1f00, DS:d68d + 2m), paid
  as Thrown Weapons when the battle mode ends (s014 0e76:0694), which in a dungeon is at the stairs or a door;
  his byte was 0x1f in DKSAVEEY and the log shows his Stone-tar.
- 2026-11-22 (the audit, three passes by three reviewers who had not seen the fixes: every hook and call into
  the game by the game's states; every write into its memory and what must stay true of it; every past fix,
  whether at its cause. Their reports are kept out of the repository; what each found and what was done about it
  is logged here as it is done). From the third, the first: the item list was one of three. cards_shutdown frees
  DARKLAND.LST's block, the places' (loc_buf, 0x19e pointers at 2e38:10be) and the city's record (city_rec,
  DS:9785), and the battle's sheet reads and frees all three again; that afternoon's fix kept the 46-byte item
  records alone. Seen with udefs in his DKSAVEEY: 27 of the 414 place records were still places in the battle.
  And the fix offmap wrote through them: offmap_frame ran in a battle too (every 64th frame: any word of the
  battle's memory that lay where a place's column had been and read 327..335, or its row 930..939, was set to 326
  or 929); minechests asked the same record whether the place is a mine; the margins, the reputation watch and
  the save's name each had their own "last reading stands". Now mem_free_hook keeps each of the three whole as
  the game frees it (KEPT_LST_SEG, KEPT_LOC_SEG, KEPT_CITY_SEG in runtime/hle.h: 39 KB of the 47 KB free under
  the BIOS font), moves the pointers that led into it (the saints' and formulae's names too: 2e38:183e, 3bc4,
  1736, 36ec), and refuses a free of what it keeps. 414 of 414 after, through a load into the battle, its sheet
  and a load out. CK_PLACES watches the places beside CK_ITEMLIST; release_check's udefs step asks for both.
  gamemodel.c's "not in memory in a battle" guard is gone. Corrected here: the entry above of the same day says
  release_check "saves and loads there" in its battle; it does not (a fight allows no save): that door is tried
  by hand only, still.
- 2026-11-22 (the audit, second entry: the party's events over a battle). The third pass counted the doors of one
  cause: the event table swapped with EVENTS.TMP for a battle, met at the battle's end (1.2), a dead member's
  contract, a sheet in a battle (1.4.7), a save in a battle (1.5.1-RC7), each closed where it was met, by a list of
  kinds (0x4a, later 0x48 and 0x49), 48 records at most; and it found the list itself losing what the original
  keeps: a saint's help for the whole party prayed for from a sheet in a battle (kinds 0x4b, 0x4f, 0x5d, owner -2)
  was taken out of the game's write and never carried over. Tried then with potions, armorday, greatpower,
  battlesaints and sainttexts off: a Truesight drunk from the sheet in a battle is in EVENTS.TMP when the sheet
  closes, in a save made there and loaded, and back in the table when the battle is over. The original loses
  nothing: its battle code makes no event (s002..s028 call nothing of s029), and its sheet and its save bring the
  file in and write it back. Every loss had been Darklands+'s own: events made in the battle's empty table outside
  that bracket (the potion tray), then the keeping that was built round them. So the switch battlesaints claimed a
  fault the original does not have, and is gone (retired key); 94 fixes with loadsum, below.
  The cause named: Darklands+ reads and makes events while the game has them in a file. The fix at the cause
  (mods/fixes.c, "the party's events over a battle"): while the cards are closed the table holds the party's
  events, a copy of the file in memory of Darklands+'s own (EV_MIRROR_SEG, D000: no DOS block), brought in after
  cards_shutdown, after battle_sheet and after the save in a battle, and written back to the file (the table
  emptied, as the game's sheet does on closing) before world_load or that save reads it. Every reader and maker
  finds the table as on a card. Taken out with it: the potions kept at cards_shutdown, battle_keeps / battle_kept /
  battle_put_back and their 48, the sheet's taking aside, the save's rewriting of the file, the file readers in
  boosts_running, the effects and the dwarf relic's note, fixes_world_open, the mend after every battle (the mend
  once after a load stays, its notice no longer calling every leftover a bug fixed: the game's own roster leaves
  one behind with a member left at an inn). While the game swaps, fixes_events_steady() is 0 and the readers that
  draw conclusions wait (losses.c: the false "Lost permanently" of a frame inside cards_shutdown, pass 1's fourth).
  Seen in his DKSAVEEY: 63 events in the table for the battle; an Ironarm from the tray the 64th; the sheet opened
  and closed, a quicksave there and its load: 64 each time, the file the same. CK_BOOST is now in mods/checks.c,
  armed as CK_LOAD is (right once since the load), so a member taken back from an inn or F12's editor raise no
  false line; a mod's attrmax holds the current value as the game's cards do.
- 2026-11-22 (the audit, third entry: what else the three passes found and what was done).
  "A battle" was run_battle (s029 0e76:2278) to pack.c, loot.c and saves.c; a castle, mine or dungeon and every
  battle taken up from a save start from the main loop (root 03a1:000a) and never pass it (passes 1 and 2, each on
  its own). So the hidden ammunition was not handed out there (a shooter could not shoot), the thrown weapons not
  noted, and a member fallen there had his spare things given to the living when struck off, though his pile held
  them already. Now the ammunition and the note hang on battle_run (s002 0e76:0000: every battle), and "not in a
  battle" is fixes_cards_down(). Not done: the autosave before a battle is still a card's fight only (a save at
  every door of a dungeon is his to want or not).
  A switch missing from the settings took its Darklands+ default whatever preset stood: under "Play the original
  game" every fix added since was on (fixes.c reads the key at the start). options_fill_new gives a missing switch
  the value of the preset the held switches agree on.
  The stand-in leader (the best member speaks) was written into a save made meanwhile: saves.c save_hook, which
  every save passes, writes the player's own.
  The load: two paths of the original change what is in use without equip_weight (worn-out armour thrown away,
  s019 1ef8:0914; the last thrown weapon, use_ammo s014 18bf:0c48): the new fix loadsum sums it again there, so
  CK_LOAD has nothing true to complain of (the robbery's off-by-one is closed by itemcount).
  The store's ingredients joined to 255 a row where the game holds a signed char: 127, as the store's other limits.
  A card text grown by politics.c after the 900-byte guard: held under 900. thrown_last cleared at a load. F12's
  editor: the nickname's 11th byte was the weapon oils' (10 now); a quality set on a thing in use takes the
  member's copy along (gm_item_set_quality).
  Left open, by the reports' own account or mine: (1) a save at a card's text starts the card again (the
  original's; fruitonce and footpadsonce each close one handler of many): one rule in save_hook would close all,
  and change how saving at a text works: his to decide. (2) events[-1] (bounty, teachers, bountybrief fixed at
  three callers): no watch on the interrupt table yet. (3) noselection guards 7 of the 24 routines that read the
  chosen member: the other 17 not read. (4) carditems: the stated cause does not follow from the game's code;
  the true path unknown. (5) Fifteen tests under port/tools that no gate runs. (6) The save in a battle is tried
  by hand, not by release_check (a fight allows no save). (7) Coverage: pass 1 read about 270 of 1064 entries line
  by line and cleared 675 by the state they run in; pass 2 read 337 of 779 write sites in their whole function;
  pass 3 traced 17 of the 95 fixes through the game's code and took 78 on their comments with the hook point
  checked.
- 2026-11-22 (the audit, round 2: five more reviewers on what round 1 had not read line by line; the first back,
  on the 78 fixes round 1 took on their comments: 43 true and fixed, 30 read only in part, each named in its
  report). Found and put right: (1) tomb: the comment, the switch's text, the changelog and the answer given on
  issue 29 said the original let a party that got out by the way out keep the thing. It did not: s108 1838:02b0
  sends results 0, 1 and 2 through one tail (the text, then cur_loc = -1), so the original gave nothing after a
  victory or an escape and gave the thing after a defeat. What the fix does (kept by a victory or an escape, lost
  with the fight) stands as the maintainer's choice, told to him as such; the texts now say what the original did.
  (2) cardslips' Hanse guide: the game does let him go, at the next step out of town ($NPCEx00, by the kind 0x47
  notice the office makes: s029 0e76:4aa4 (7)); "and then stayed on" was false. And the hook found "the guide" by
  the nickname Hanse alone: a member of the player's so called would have been struck off. Now by the game's own
  record (hired_member: the notice, or the Schulz's contract). (3) Hooks asking ns_unwinding after
  hook_call_original missed a load made from the game's own loop (the native cards off: only ns_call set it):
  hooks.c now sets it whenever the original comes back unwinding. Left open: bounty may free the wrong patron's
  reward when two wait in one town (plausible); armorday's text overstates with fireward off.
- 2026-11-22 (the audit, round 2, the other four reviewers: today's changes read by one who had not made them;
  every hook of fixes.c read in full, 208 pairs; the 469 hooks and calls outside it that round 1 had cleared by
  state; the 442 writes round 1 had classed from the statement, 251 of them now read in their function, and the
  open leads). What they found and what was done.
  In today's own changes: (1) options_fill_new told "new" from "never written" by the keys held, and would have
  switched 29 additions off for a setup that happened to match Purist: now only for a preset on record (the
  original's flag; "preset" = Purist with every held switch still at its values). (2) The preset changed from Full
  Original to Darklands+ in a battle left the table empty: fixes_original_changed brings it in (ev_bracket tells
  the game's own sheet or save; ev_bring does nothing when already brought). (3) CK_LOAD only with loadsum and
  itemcount on. (4) The frames that rebuild from the events (potions, armorday, greatpower) and those that move a
  task (altar, rkcastle; these not while the cards are closed either) wait while the table is swapped. (5)
  battledead had nothing left to do (party_delete finds the events itself now): retired. carditems likewise, its
  stated cause never in the original and the item list now always whole: retired, CK_ITEMLIST the watch. 92
  fixes. (6) load_mend under loadsum (it had put the original's own leftovers right with no switch). (7) A store
  row joined past 127 before is split again.
  In the fixes' hooks: (8) castlefreeze went on turning any roll equal to the gate room into 0 after breaking the
  loop: the direction rolls after it (15af:2082, 2097: roll_dice (1, 4) - 1) came out -1 one time in four, and
  15af:3372 never ends with that: the freeze the fix is for. Now only the roll that returns to 15af:1fda. (9)
  cardopts ran twice once a fix had been switched off and on (it goes outermost; the native cards' hook calls
  fixes_card_opts too) and w04_options scrambled $CityW04: one caller now. (10) debtnote held its changed +0x2a
  across the routine's text, where a save took it: put back before the text. (11) The saints' armour and fire
  hooks and thrown_end wrote after an original that may have come back unwinding (a load at its message): they
  return then. All 183 hooked functions: far or near as their hooks take them, no argument read by SP after the
  original.
  Outside fixes.c: (12) ingr_store_hook lowered the store's count but not the count cache_save writes
  (GV_cache_n_shown): a store with an ingredient in two rows was written back with a stale copy of its last row
  at every look, on by default since 1.4. (13) no_papers_hook called the original again without putting the
  return address back (a crash at the errand's card; the switch is new and off). (14) levelwalk's U key returned
  without the return when it took the key. (15) rich_start wrote its florins into a game loaded at the opening
  card. (16) levelwalk walk_to marked every member as walking in a fight too: their orders dropped. (17)
  orders.c best_attack asked path_find for a way to a foe without making the foe the target (path_find answers
  no for a place anyone else stands on): a shot ordered where the attack was meant. (18) A refused click on a foe
  left the order mode on attack. (19) A request deferred from the party panel in a battle's last tick ran at the
  next menu bar, a card's: ns_defer_here runs only in the state it was asked in. (20) ways.c, battlerules.c and
  pathfix.c forgot a battle only when a frame saw none: now at battle_run and at a load. (21) "No such event":
  the four bytes before the table lead to an empty record of Darklands+'s own (noevent_frame), so a read of
  events[-1] finds nothing and a write or a free harms nothing; 51 calls listed in the report.
  Sound, by their reading: the mirror and the kept blocks on every path enumerated (all 8 uses of EVENTS.TMP,
  every caller of world_load, cards_shutdown, cards_startup, battle_sheet, battle_run, the save); their memory
  unused by anything in runtime/; all 414 places pass CK_PLACES; no stale target or record number in the order
  writes of ways.c, orders.c, battlerules.c; noselection's other 17 routines (the one write through member -1
  is dissolve's); aftermath_end.
  Open, each with its reason: a save at a card's text starts the card again (some 180 handlers at most give
  before their text, every patron's payment among them; and the fetched item's reward card can free another
  event after such a load): one rule in save_hook, his decision, asked twice. bounty with two patrons' rewards in
  one town (plausible). pack.c's hand-out at a battle taken up from a save uses give_one, not give_safe
  (plausible). cardview.c common_step without the game's text keeper 150b:13f2 (Party Info's native screen off:
  not the default). The fifteen tests no gate runs. The save in a battle and the preset change in a battle are
  not tried by release_check. Not read by anyone: sainttexts.c's saints, travel.c's planner, encounters.c's
  loader, 188 writes of the native views and small files, 30 of the fixes' claims in part.
- 2026-11-22 (his "do whatever you think is best" on the two things asked). (1) The save at a card's text: the
  new fix savetext (93 fixes), in mods/saves.c where every save passes: at a card a save is made only while the
  card's choices are up (assist_in_card_menu), the state a load brings back; a quicksave, an autosave, an Ironman
  save or the menu's Save (its name taken at once) asked for at any other moment at a card waits for the choices
  or the map, with a notice. Seen at the opening card: asked at its text, "The game will be saved when this text
  is closed", then "Game saved" as its four choices came up; asked at the choices, saved at once; that save loaded
  stands at the same card. fruitonce and footpadsonce stay (each also holds its gain back for a player with this
  one off). Not closed by it: a save at the choices of a card whose first statements give something before the
  choices are shown (none found; not searched for). (2) The tomb left as it is; the correction posted on pull
  request 29. Also from the open list: bounty picks the reward of the office the party came from (saved_card, as
  bountybrief tells the offices), not the last in the table; tools/run_tests.py builds and runs the ten C tests
  by the command in each one's header, and release_check runs it first (all ten pass on today's code; the five
  Python ones need a game folder and a save, by hand still); armorday's text no longer claims the fire
  protection's leftover armour, which fireward leaves out.
- 2026-11-23, round 3 of the audit (his "quash as many bugs as AI-ly possible"): eight reviewers on what no one had
  read (the saints, travel and the encounter files, the native screens, the fixes' remaining claims, the interface
  code, the runtime layer, saving and loading, the battle helpers' logic), three soak tests, a memory-checked build
  through release_check (no error reported in Darklands+'s own code; only its drawing-time step fails, as that
  build is slow). Reports in the session's scratchpad, audit/round3_*.txt. Fixed at the cause, each with its
  reasoning at the code:
  Saves: written_bad reads every save back before a notice or any prune (the game's save checks no write; a short
  file rotated good saves out); saves_party_ended asks the party (no member left, or battle result 5), not the exit
  code (the game's fatal stops leave by exit 0: Honor Roll and an Ironman save deleted); the BIOS tick word starts
  at the time of day (the party's seed is its low word at the click; from 0 every seed fell in a few hundred, and
  every companion file is keyed by the seed); a player's save name never begins as Darklands+'s kinds do
  (players_name); Ironman's quit and close wait for a card's text; the menu's Save asks text_now before its page
  (after it the choices read as down); state_fp, taken at card_load: a save at a card's choices waits until the
  card is left when the party has changed since it began ($Revol12, $Revol11, $Drago00, $Dunge00 ...), and past an
  encounter's first choice until it is over (encounters_past_start); savecheck_sav_tail: a fresh save's store-room
  tail at least CACHE.TMP's size; battleend, autopause and orders forget a battle at battle_run and a load.
  Battle: door_lock (a locked door clicked is not door_toggle'd: the leader is chosen there); take_exit_hook knows it
  runs inside combat_keys and takes the party once (the game calls it once a member); the carried go first, so
  leave_level runs once with the fight on (pursuers follow) and combat_recover wakes them when none follows; U picks
  only stairs seen; a pending level-map click is the last battle's; costspill (new fix) keeps 20a5:7900 over
  1db7:0028 for enemies 11 and 12; Halt kept (halted[]): a halted member walks nowhere of its own; set_kind resets
  the attack rate cache (DS:d493) with every kind written; a foe fallen while a member walked to it ends the walk;
  a kept stance survives a click that gives no order; ways_players_mark: no other foe for the player's own mark;
  can_fire_hook hands hidden ammunition to a weapon readied inside a place; thrown_note after each loot screen of a
  place; exitleft keeps the gone out of the marching order; battleend's ticks are 60 a second (its text said 18.2).
  Saints: hook_return_far/near leave SP alone while unwinding; the saints' wrappers stop at an unwound call; relics
  count one discount a question, for the saint asked (fix pauls answered Paul the Simple through Paul's hooked
  function: 2d); saint_armour and fire_saint take the leader for "Pray now" with prayfor off; pot_all to 300;
  the chance shown is the game's number + 1; nine saints' "all weapon skills" noted (saintinfo, SAINTS.md).
  Screens: ingr_pile at the loot screen's first draw (it read the last screen's freed pile), the caller's count
  lowered on a join; inn_key_hook (s038 02f8 strikes a member off by itself: spares_off); gm_pack_holder leaves
  the fallen (status 3) out of the pack; pilefull (new fix); Undo keeps its entry until the things are back; the
  inn's page steps its arrows itself.
  Elsewhere: Go to stays off a card reached from the map (an encounter by a town) and off lines that do more than
  leave; encounter option flags per menu; reputation to the town the party is in; a journey's first step may carry
  the old click's key; the 20:00 camp after an encounter step; the step's AX put back; find-first gives the size
  of the file a read will get (a data mod's); day counts without leap years (quests.cpp, lodgeview.c); the battle
  log and the foes' list restart at a load; the timer rate the game sets is logged.
  Open, by their reports: Windows paths with non-ASCII letters; the hour the clocks go back; settings keys of 64
  characters; the keyboard layout; key releases lost under a Darklands+ window; two copies on one folder; the line
  clipper's overflow; the cachefile layout the original rewrites wrongly; the Schulz's reward (cardslips);
  oilweapons' and knightdead's wording; the native screens' shared queue; the tray's drink and Split without
  give_safe's bookkeeping in a battle; the load preview's weight of relics; indexes kept across a mods reload;
  hit chances shown without the rate gate; Simmern on Geldern's tile (the original's data).
- 2026-11-23, the same round, second part (release_check passed on dbff504 first). cachefile writes the original's
  own header (99, the count, offsets, zeros to 101, stores in order) up to 49 stores, so cache_save's walk by the
  first offset's low byte as a signed char is right (0x65; 198's 0xC6 is -58), and an unreadable store is written
  back empty (cache_dropped) instead of refusing every write; test_cachefile.c checks both. pack.c record_gone_hook
  on item_delete (every record lost goes through it) keeps DS:c7cd, the Eater-Water's record number, in a battle;
  give_safe no longer adjusts it itself. Windows: res/darklands.manifest, activeCodePage UTF-8. platform_sdl.c
  game_down[]: a key the game was told is down gets its release whoever takes the event, and all at focus loss.
  config.c: keys 200, values 520, the file written beside and renamed. dos.c dos_local: a fixed offset from UTC
  taken at the start. gm_member_worn_id for ns_load_with. overlay.cpp rebuilds its keys at datamods_generation and
  bounds the saint and formula indexes; ui_town_offers bounds the town.
- 2026-11-22 (skill gains missing after a spider fight or on leaving the field): the aftermath was built only
  when victory opened a loot pile. Weapon practice can also settle on retreat or when the party leaves a
  dungeon after collecting its loot. The report now reads the party after combat settles, using the last
  dungeon report as its baseline. An empty victory uses the timed notice. The release check exercises a
  spider victory, a retreat, a dungeon exit after looting and a victory without practice. Defeats and
  surrenders keep their original result notices.
- 2026-11-23, after RC23: savetext's state_fp is taken at a card's FIRST card_load only (card_fresh: set at
  card_menu, off the cards, and at a load). The opening card $Party02 rolls the purse and the date between its two
  card_loads (s054 18d3:0008), so a save at its choices was made at once and each load rolled again (seen: saved
  on 24 May, loaded on 27). Tried: held there until the card is left; ten town cards in a row each saved at once.
  Wine works again here with TMPDIR unset (nctl.py does it): Wine 9.0 aborts at start ("free(): invalid pointer")
  with TMPDIR=/tmp. RC23 run under Wine from a folder named "Jürgen": the module made, a quickstart, a held
  quicksave made at the text's close, loaded.
- 2026-11-23, round 4 (one reviewer on everything changed since RC21, report audit/round4_regressions.txt): no crash,
  spoilt save or lasting wrong state; five findings, all fixed. saves.c: hold_why says why a save waits (a text, a
  card that has given, an encounter past its first choice) in every notice, the Ironman quit and close among them;
  menubar_hook asks text_now only with a save waiting (it runs at every poll, and state_fp hashes some 3 KB);
  card_fresh also when the card number (DS:a772) changes. ways.c: Halt ends with the fight on the field
  (real_combat off: a place is one battle_run). pack.c ensure_one takes ammunition only from giver_ok members. fixes.c
  exitleft strikes the gone out of the marching order itself (exitleft_prune) instead of calling the game's rebuild,
  which shows a message for a second with the gone shown as absent. MODDING.md: an option flag before the first menu
  does nothing. Fuzzed under ASan/UBSan, nothing found: savecheck (32,000 mutated saves and battle files of his),
  the encounter loader (300 rounds of mutated files), cache_rewrite (2 million random files).
- 2026-11-23, the village's robber knight (round 3's fix_claims finding 8): lordreward's village_reward_hook on
  $Villa02 option 0 (s077 1996:01ae): with ev24_at_owner (3, 9, the village) it sends the card to $Raubi01 with
  saved_card 0x86 and saved_card2 9, as the council's 19df:0298 does for giver 10. Tried in a session (DKSAVED5,
  Nürnberg as the village): the quest given by the game's own 1462:10b6 (9, castle 100, ...), the knight's fall by
  ev8_resolve (3, 100) - a kind 0x24 reward for owner 9 at the village; a member named Schulz with a kind 0x43
  contract; ujumpcard 0x86, the news asked: $Raubi01's text 1, the reward freed, the Schulz gone (raubi_hook's
  branch, unreachable until now), the village's card back, a second ask the plain news.
- 2026-11-24, Find teachers (his report, two pictures): the rows drew their icons with the outer window's draw list, but a
  ScrollY table is a child window: the icons were unclipped (over the header) and under the child (gone beneath the
  hovered row's highlight); ui_dim_since ran on that list's indexes, so other columns' text was never dimmed. Now the
  table's own list, dimmed per cell (each column is a channel of its own), an ImGuiListClipper over the filtered rows
  (every row's rich text was laid out each frame), the tooltip beside the window level with the row (DelayShort, none
  while the wheel turns). ui_teach_who words the nobody case itself (margins.cpp shared it). Tried in a session
  (DKSAVE32, map.offers_unvisited, new test command uteachers): hovered row intact, header clean after six wheel
  notches, tooltip at the window's right.
