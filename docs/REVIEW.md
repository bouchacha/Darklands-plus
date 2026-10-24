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
- one value for each shared color (the lectern ink, the blue of Darklands+ additions, the rubric red).

**Words.** `python3 port/tools/copy_catalog.py > docs/COPY.md`, read as a player who never played the original:
literal verbs (what really happens), no technical talk, jargon explained once where it first appears, names
that say what is inside, the exact F12 paths, American spelling, "world map (M)" / "overland view" / "level
map". For every switch removed, merged or renamed since the last pass: search every text (COPY.md, the guide,
the welcome, the tips, README) for anything that still calls it optional or uses its old name. Long lines are
wrapped (ImGui's BulletText and TextDisabled do not wrap).

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
