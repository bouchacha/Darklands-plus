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
  (104k); the ornaments are 62k of alchemy's 89k (display.ornate off: 27k) - a textured braid would cut them, not
  done. Not checkable here: Go to University's "cannot find the way from this card" (not reproduced from the
  player's save at any of 30 places, nor after the university's own cards; the log now names the card, and the last
  session's log is kept as darklands_port_previous.log).
