# The review pass

A pass over the whole of Darklands+, run from time to time, for three standing aims: code a newcomer can find
their way in, one style in words and pictures, and fewer, plainer switches. It looks at the whole before the
parts: a pass that only works through lists finds what is wrong with each item, not what is wrong between them
(two pictures for one thing, a text that a later change made untrue, a switch that only matters with another).
Every pass does all of it; each ends with a line in the log at the bottom, so the next one knows where the
last one stopped and what it missed.

## 1. The whole, first: walk it as each kind of reader

Play through, as a player would, with the one-time windows shown (the test sessions hide them: start with
`DL_TIPS=1 DL_WELCOME=1 DL_NEWS=1 DL_MT32_PROMPT=1`), and take a screenshot at each step:

- **A new player**: the first start (the MT-32 window, the intro, the welcome on the start screen, the guide's
  note), a new party, the first town, the first card, the first battle (its tips), the loot, the inn, travel,
  saving, quitting. Is every step explained where it happens, once, in plain words? Is anything said twice in
  two ways?
- **A player of the original** on the Full Original and Purist presets: does anything of Darklands+ still show?
- **A returning player** of an earlier version: what is new, and does anything they knew move without a word?
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
