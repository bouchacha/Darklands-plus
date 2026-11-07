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
