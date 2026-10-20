# Modding the native port

The native port (`port/`) reads mods from folders, so the game's own files are never changed and
every mod can be switched off again. Everything here is done from the game's F12 menu; no tools
are needed.

## Mod folders

A mod is a folder in the game folder's `mods` folder:

```
<game>/mods/My mod/
    mod.txt                 first line: the name shown in F12 > Settings > Mods; the rest: a description
    DARKLAND.LST            any game file, replaced whole (items, saints, cities, enemies ...)
    PICS/CASTLE1.PIC        one picture
    MSGFILES/$MAINS01.TXT   one card file as text (or .MSG); the other cards stay as they are
    A00C.CAT/<entry>        one file inside a .CAT catalogue (also BC, LCASTLE)
    rules.ini               weapon numbers (below)
    art/  sounds/           replacement pictures (PNG) and sounds (WAV/OGG), as in the game folder
    music/                  recordings that replace the game's pieces of music (below)
```

Tick the mod in F12 > Settings > Mods. Several mods can be on at once; where two change the same file the
folder that comes later in the alphabet wins (name them `10 base`, `20 tweaks` ... to set the
order). Only reading is redirected: saves and settings stay in the game folder.

When the game folder cannot be written (Windows: under Program Files, where GOG Galaxy installs
games), the `mods`, `art` and `sounds` folders are in `%LOCALAPPDATA%\Darklands+` instead, and
the game's saves go to Windows' VirtualStore copy of the game folder; F12 > Settings > About shows where.

Card texts, pictures, sounds and weapon numbers change at once; files the game reads when it
starts (the item list, saints, cities ...) need a restart.

Implementation: `port/mods/datamods.c` (redirect of DOS opens for reading, catalogue merging into
`mods/.cache/`), `port/ui/rules.cpp` (rules.ini).

## Card texts

F12 > Settings > Mods > "Export the card texts" writes all 419 card files of MSGFILES as text into
`mods/_export/MSGFILES/`. Copy the ones to change into your mod's `MSGFILES` folder, keeping the
name. The format:

```
# comment lines start with #
=== card 0: 4 7 0 240 0 ===
Looking down the main street of $PlaceName, you set off toward...
\14\14\15...\1dthe $citySquare, the political center of $PlaceName.
\14\15...\1dthe $fortress, the great fortress overlooking the city.
```

Each card of the file starts with a `=== card N: y x 0 max_x 0 ===` line (the text box's top and
left edge and its right edge). The text follows as it is; bytes the game uses for layout are
written `\xx` in hex:

| bytes | meaning |
|---|---|
| `\14` | paragraph break |
| `\15` | an option (`\15...\1d` then its text) |
| `\16` `\10` `\06` | an option that opens the saints, the potions, or starts a battle |
| `\01` ... `\13` | a part the game shows or hides |
| `\1f`, `{`, `\|` | a-umlaut, o-umlaut, u-umlaut |
| `$PlaceName`, `$Inn`, `$Money1` ... | filled in by the game |

The option *order* matters: the card's script refers to options by number, so keep the options,
change their words. The text form converts back byte-for-byte (all 419 files were checked).

## Weapon numbers (rules.ini)

F12 > Records > Weapons lists the battle numbers of every weapon; "Write these numbers into a
mod folder" makes a `rules.ini` with all of them to start from.

```ini
[weapon Short Sword]      ; an item name of DARKLAND.LST, or [weapon 3] by type number
skill = edged             ; edged, impact, flails, polearms, thrown, bows, missile devices
damage = 9
penetration = 4
hands = 1
time = 40                 ; for a blow; lower is quicker
reload = 0                ; missile weapons
skill_wanted = 16         ; each point short: -2 attack, -2 defence
strength = 13-28          ; below: -1 attack, -3 defence per point, softer blows; above: harder blows
```

The numbers live in tables of the game's code (far segment 20a5, read by the battle module s014
when a weapon is readied). The port keeps the game's own numbers and puts them back when a mod is
switched off.

## Words of the menus (text.txt)

Menus and screens the game writes from its program (shops, the inn, the sheets) are not in the
card files. A mod's `text.txt` says such lines differently, one per line:

```
Purchase an item => Ware kaufen
The Goods Merchant offers... => Der Krämer bietet an...
```

A hotkey letter and the rest of its word count as one line. "Collect the texts shown" (F12 >
Mods) writes every line the game shows into `mods/_export/texts_seen.txt`, ready to translate.
This works with crisp text on (it replaces the words where they are drawn).

## Items

F12 > Records > Items lists every item of DARKLAND.LST with its name, short name, weight,
quality, rarity and value, all editable; "Save the list into this mod folder" writes the whole
list as the mod's `DARKLAND.LST`. The game reads the list when it starts.

## Foes

F12 > Records > Enemies lists every foe of `DARKLAND.ENM` (71 kinds, 82 named foes) with its
attributes, weapon skills, armour, shield and weapons, and which weapons pierce its armour. The
attributes, skills and armour quality can be changed there; "Save the foes into this mod folder"
writes the whole file as the mod's `DARKLAND.ENM`. Foes of one kind share their numbers. The file's
layout is in `formats/dlformats/enm.py`.

## New encounters

A mod can add wilderness encounters of its own: cards like the game's, with a picture, text and
options that test the party's skills, run by Darklands+ from a plain text script
(`port/mods/encounters.c`). Darklands+ comes with one such mod, **New encounters** (sixteen of
them; tick it in F12 > Settings > Mods). Its folder is the best place to start reading:
`mods/New encounters/ENCOUNTERS/*.TXT` and `MSGFILES/$NW*.TXT`.

While a mod with an `ENCOUNTERS` folder is ticked, some of the random encounters on the travel
map are new ones. Each new encounter counts as `weight` more entries of the game's own table of
encounters for the terrain (9 to 29 entries), so with a weight of 1 it is about as common as one
of the game's own; F12 > Settings > Mods sets an overall percentage, and none comes twice within a few days.
A small red seal in the corner of the card marks a new encounter (its tooltip names it). Not in
the Buggy Purist preset (the game as it was).

An encounter is two files: its text, `MSGFILES/<CARD>.TXT` in the card text format above (block 0
is usually the first menu), and its script, `ENCOUNTERS/<NAME>.TXT`:

```
# The gallows oak: a hanged thief, and the mandrake that grows beneath him.
name     The gallows oak             # shown in the seal's tooltip
card     $NwGal00                    # the MSGFILES text (at most 8 letters)
picture  XTREE                       # PICS\XTREE.PIC (pictures starting with X get white text)
terrain  farmland plains woods forest
when     any                         # day, night or any
weight   2                           # how many table entries it counts as (0 to 100)
only     month >= 4 and month <= 10  # (optional) a condition for it to come up at all
vars     tries, found                # (optional) numbers the script keeps while it runs

[start]                              # the first section is where the card starts
block    0                           # this section is a menu: the options of text block 0
chosen   bestskill(Alchemy)          # $ChosenOneName (and $he/$his/...) on entering
option 0                             # the first option line of the block
  who    bestskill(Alchemy)          # who is tested (shown next to the chance)
  chance clamp(5, 90, skill(who, Alchemy) + attr(who, Int) / 3 + night * 15 - 5)
  ok     text 1; additem who, 0x54; practice who, Alchemy, 1, 10, 0
         hours 1; exit               # indented lines go on with the same actions
  fail   text 2; injure who, 1, 1, 10; hours 1; exit
option 1
  chance assured                     # no roll: "Success is assured." (also none, unknown, combat)
  ok     hours 2; if roll(25) goto bailiff
         text 3; attr party, DF, skill(who, Religion) / 5 + 2; exit

[bailiff]                            # a section without a block: actions only
do       text 4; reputation -1, -3; exit
```

**Sections.** `[name]` starts one. `block N` makes it a menu (text block N with its options);
`picture` changes the picture on entering it; `do` lists actions run on entering (before the
menu); `chosen` sets `$ChosenOneName`; `saints a, b, c, d` and `items a, b, c, d` are the saints
and potions its saint and potion options offer (numbers from DARKLAND.LST; up to four). A section
without a block runs its `do` and then returns to the last menu (or the map) unless it went
somewhere else; going back to a menu that way does not run that menu's `do` and `chosen` again
(a `goto` to it does).

**Options** are numbered by their order in the text block (each `\15`, `\16`, `\10`, `\06` line is
the next number). `option N saint` is a line that opens the saints (`\16`), `option N potion` one
that opens the potions (`\10`). Keys:

| key | |
|---|---|
| `only EXPR` | the line is hidden unless EXPR |
| `need EXPR` | the line is greyed unless EXPR |
| `who EXPR` | who is tested: a member, `party` (everyone) or `leader` (the default) |
| `chance EXPR` | the chance in percent; the game rolls `random(100) <= chance`, as its own cards do. `none`, `assured`, `unknown`, `combat`: no roll, the game's own wording |
| `potion CODE, EXPR` | (potion options) the chance with that potion; `who` is the one throwing it, `q` its quality; it is used up |
| `ok ACTIONS` | on success (or when there is no roll) |
| `fail ACTIONS` | on failure |

A saint option runs the game's own prayer (the Divine Favor is spent, the saint's chance shown);
`ok` if it is answered, `fail` if not, nothing if the prayer is called off. After an option the
menu comes back unless its actions went elsewhere.

**Actions**, separated by `;`:

| action | |
|---|---|
| `text N` | show text block N (and wait for a key) |
| `goto SECTION`, `exit` | go to another section; back to the map |
| `card N` | on to the game's card N (in a town: 0x13 the church district ...) |
| `tocity` | end at the nearest city's gate |
| `if EXPR goto A else goto B` | |
| `if EXPR then ACTION else ACTION` | an action, or several in `{ ... }` |
| `set NAME = EXPR` | a number declared with `vars` |
| `hours N`, `days N`, `until HOUR` | let time pass |
| `practice WHO, SKILL, MODE, PCT, LEVEL` | the game's chance to improve a skill by use (as its cards: success `1, 10, 0`, failure `0, 10, 0`) |
| `skill WHO, K, D`, `attr WHO, K, D`, `attrmax WHO, K, D`, `setattr WHO, K, V` | change skills and attributes. With `party`, D is worked out for each member (`who` is that member) |
| `injure WHO, A, B, MULT` | the game's wounds (its boars: `1, 1, 10`) |
| `pay PF`, `gain PF`, `money1 PF`, `money2 PF` | money in pfennigs (240 to the florin); `money1` and `money2` set `$Money1`, `$Money2` |
| `heal WHO, ATTR, PCT` | that share of what was lost of an attribute comes back (the game's healing) |
| `number1 N`, `number2 N` | `$Number1`, `$Number2` |
| `additem WHO, CODE[, QUALITY]`, `takeitem WHO, CODE` | items (`$ItemName` names the last added) |
| `loseitems WHO`, `keepitems WHO, PCT`, `removeitems WHO, MASK` | the game's losses: robbed (with `party` also the purse), each stack kept with PCT % |
| `fame K` | the game's reward of fame (1 to 5: 3, 10, 25, 64, 200) |
| `reputation A, B` | the nearest city's reputation (the game's random change: `1, 3` up, `-1, -3` down) |
| `battle FIELD, ENEMY, QUALITY, COUNT[, ENEMY, QUALITY, COUNT ...]` | a battle; `result` is then `Won`, `Fled`, `Beaten` or `Surrendered` |
| `rejoin`, `dropaway` | after fleeing: members left behind come back, or are lost |
| `kill WHO` | a member is lost for good (as the game's harshest cards do; never the last one) |
| `chosen WHO`, `chosen2 WHO`, `saintname S`, `itemname CODE` | `$ChosenOneName`, `$ChosenTwoName`, `$NamedOneName`, `$ItemName` |
| `enable N`, `disable N`, `hide N` | an option of the menu, for the rest of the encounter |
| `music` | the game's ambush music |

**Values.** Numbers (`0x54` too), `+ - * / %`, comparisons, `and or not`, and:

| | |
|---|---|
| members | `leader`, `party`, `who`, `chosen`, `bestskill(K)`, `worstskill(K)`, `bestattr(K)`, `worstattr(K)`, `bestskill2(K1, K2)`, `bestskillsum(K1, K2)`, `bestattrsum(A1, A2)`, `bestattrskill(A, K)`, `spotter` (best Perception + Woodwise), `slowest` (the slowest with their load) |
| one member | `skill(W, K)`, `attr(W, K)`, `attrmax(W, K)`, `able(W)`, `has(W, CODE)`, `count(W, CODE)`, `quality(W, CODE)` (`has(party, CODE)`: anyone) |
| the party | `wounds(ATTR)` (what the members present have lost of it, together), `standing` (the reputation here, or in the nearest town), `maxskill(K)`, `minskill(K)`, `avgskill(K)`, `maxattr(K)`, `minattr(K)`, `avgattr(K)`, `maxskill2(K1, K2)`, `maxskillsum(K1, K2)`, `maxattrsum`, `maxattrskill`, `spotvalue`, `avgagility`, `minagility` (agility with the load), `slowspeed`, `lore` (1-10, the game's measure of the party's strength), `fame`, `money` (pfennigs), `size` (members present), `members`, `women`, `mounted` |
| the world | `daylight`, `night`, `hour`, `day` (1-31), `month` (1-12), `year`, `terrain`, `difficulty`, `event(K)` |
| rolls | `random(N)` (0..N-1), `roll(C)` (a check of C %), `clamp(LO, HI, V)`, `min`, `max`, `abs` |
| after | `result` (a battle), `q`, `item`, `saint` |
| names | skills `Edged Impact Flails Polearms Thrown Bows Missile Alchemy Religion Virtue SpeakCommon Latin ReadWrite Healing Artifice Stealth Streetwise Riding Woodwise`; attributes `End Str Agl Per Int Cha DF` (or in full); enemies `Bandit BrigandSergeant RobberCaptain Mercenary Raubritter Villager Friar Huntsman Wolf Boar Bear Spider Kobold Gargoyle Schrat Tatzelwurm` and more (see `CONSTS` in encounters.c), or any DARKLAND.ENM number; battlefields `Wilds Surrounded Clearing` (`Cave` holds one enemy only) or another of the game's wilderness battlefields (0x2e-0x55, 0x61-0x63, 0x6f-0x79) |

Terrains: `marsh woods farmland plains forest pinewood hills mountains road`, `wild` (all but
farmland and road), `any`, or the map's terrain numbers.

To keep a new encounter in step with the game's own, take its numbers from them: the chances of
the game's cards are clamped (usually 1..99) sums such as `(4 * Cha + SpeakCommon) / 5` for
talking, `avgskill(Woodwise) + avgagility * 2 / 3` for sneaking, `minskill(Riding) * 2` for
outriding; their battles are `random(5) + 3` men of quality `lore / 4 + 1` with a leader of
quality `lore % 3 + 1`; their rewards are a little fame, reputation and Divine Favor rather than
money.

Errors are reported with the file and line in `darklands_port.log` (and the encounter is left
out); `DL_ENC_LOG=1` logs every roll. The script is checked against its card text when that is a
mod's `MSGFILES/<CARD>.TXT`: each `block` and `text` must exist, each option must have its line
in the block, and a `\16` line must be a `saint` option and a `\10` line a `potion` one. The
game holds at most 25 text blocks of under 6000 bytes in all, with up to 10 option lines in a
block. What can only be known while it runs is checked then: an item, saint, skill or member
out of range, or sections that lead only to one another, end the encounter with a notice
instead of upsetting the game. A battle holds at most 8 foes (a `Cave` one); groups beyond that
are cut down, and qualities are kept within 0 to 9. For testing, the `uenc:N` step of `DL_CTL` /
`DL_SCRIPT` makes the next wilderness encounter number N.

## Party figures

Select Character Image offers the game's own figures (any `PICS\F??STAT.PIC`, `A??STAT.PIC`,
`C??STAT.PIC`, as in the original) and then the human enemies' figures (`mods/looks.c`). A mod can
add one: a new group `Exx` in `E00C.CAT` (`ExxWK<weapon>.IMC`, `ExxCB<weapon>.IMC` for the same
weapons, `ExxDY.IMC`), a `TACANIM.DB` record with its buffer sizes and an enemy type in
`DARKLAND.ENM` whose image group is `Exx` (its name names the figure; its first `ENEMYPAL.DAT`
palette gives the colours). The portrait (`ExxSMALL.PIC`) and the little battle figure
(`ExxSHORT.PIC`) are made from the sprites unless the mod has its own in `PICS` (use colours
0xEB-0xF2 for the member's own colours, as the game's pictures do).

## Replacement art and sounds

`art/NAME.png` replaces the picture `PICS/NAME.PIC` (or a sprite of that name) at any resolution,
drawn at window resolution; F12 > Settings > Display can export all original pictures as templates
and show the name of the picture under the mouse. `sounds/NAME.wav` (or `.ogg`, `NAME2.wav` ...
for variations) replaces a battle sound; the names are listed in F12 > Settings > Display. Mods can
carry both folders; a mod's files win over the game folder's.

### Making a whole art pack

`port/tools/artpack.py ORIGINALS OUT` turns the exported originals (F12 > Settings > Display > "Export
the original pictures", which writes `art/original`) into a mod folder: `OUT/mod.txt` and
`OUT/art/NAME.png` at four times the size. A *style* decides what becomes of each picture; the
sample style `clown` dresses every piece of armour on the character sheet as a clown costume
and turns shields into bullseyes, which makes it easy to see that the replacer works. Styles are
plain Python functions (picture in, picture out, or None to leave it), so an image model can be
plugged in the same way. A pack made from the originals carries the game's pictures: keep it
for yourself.

### Repainting the pictures with an image model

`port/tools/aiart.py ORIGINALS OUT --backend ...` is the pipeline for a full replacement: every
card and screen picture (and, if asked, the armour, shields and weapons) goes through an image
model as image-to-image, starting from the original scaled up, so the composition stays and
the painting is new. It keeps what must not move (the parchment edge of the cards, the shapes of
the items), draws the colours towards the original's, writes `art/NAME.jpg` / `.png` at four
times the size, and makes `review.html` with each original beside its new picture.

- **Backends:** `a1111` (a local Stable Diffusion WebUI such as AUTOMATIC1111 or Forge, started
  with `--api`; any checkpoint), `comfyui` (a ComfyUI server: a plain image-to-image graph with
  `--checkpoint`, or your own graph in API format with `"{image}"`, `"{prompt}"`, `"{strength}"`
  ... placeholders), `openai` (OpenAI's image edits, `OPENAI_API_KEY`), `command` (any program,
  e.g. a diffusers script) and `test` (a filter, to try the pipeline).
- **Prompts:** one style for the whole pack (`--style`), words for each kind of picture, and a
  caption for each picture in `OUT/captions.txt`: `--write-captions` writes the file to fill in,
  `--caption-with openai` has a vision model describe each picture first.
- **Control:** `--strength` (0.3 stays close, 0.6 repaints freely), `--only NAME,...` and
  `--redo` to make chosen pictures again, `--seed-offset` for another variant, `--limit` to do a
  few at a time. The seeds come from the names, so a run can be repeated; `aiart.json` records
  what was asked for each picture.

As with `artpack.py`, a pack made from the game's pictures is for your own use.

### The sample battle sounds

`port/tools/soundpack.py OUT` makes the "Battle sounds (0 A.D. recordings)" mod that comes with
the release (off until ticked in F12 > Settings > Mods): 50 takes of the eleven battle sounds, cut from
0 A.D.'s free recordings (Wildfire Games, CC BY-SA 3.0) to start on the blow, levelled, and in
places layered (a hit on mail, the body falling after a death cry); the flask breaking and the
magic sparkle are synthesized. It fetches only the files it uses from 0 A.D.'s repository
(`--src` takes a 0 A.D. `audio` folder instead) and needs numpy and ffmpeg. The pack holds
nothing of Darklands, so it can be shared with its `CREDITS.txt`. To make a pack of your own,
change the recipes in `build()`, or drop your own files into a mod's `sounds` folder.

## Music packs

A folder `music` (in a mod, or in the folder with Darklands+'s own files) holds recordings
that the game plays in place of its own music, piece by piece. Each file is named by the
number the game asks its sound driver for, with anything after it: `10.ogg`, `11 market.ogg`,
`08 church.wav` (Ogg Vorbis, or PCM .wav of 8, 16 or 24 bits; any sample rate, mono or stereo).
Up to four files for one number are played in turn (`27 a.ogg`, `27 b.ogg` ...; the game's own
piece 27 is one of four tunes in turn).
The pieces are the same for every sound device. F12 > Settings > Display > Sound lists the numbers that
are known (a church 8, the inn 9 and 10, the market 11, the guilds and craftsmen 12, praying 13,
the city square 14, the university 15, the fortress 17, the map 19 and 20, the dungeon 22, before
a city 27 and 28, a village 41, the ending 45, the opening 47 ...; 8 to 47 in all) and names
each piece as it starts, so a pack can be made by listening.

A piece in the pack plays from its file while the synthesizer is silenced; a piece the pack
lacks plays from the synthesizer as before, so a pack can be partial. A piece that loops in the
game loops in the pack (the whole file again from its start), a piece that plays once plays
once; stopping, pausing and the music switch (Alt-M) work as in the game. Battles have no
music. Implementation: `port/mods/musicpack.c` (the numbers and how the drivers treat them are
described at its top).

Darklands+ comes with no music pack: recordings of the game's music, on an MT-32 or anything
else, are MicroProse's compositions. A pack of your own music, or one you have the rights to, can
be shared like any mod.

## Built-in changes

F12 > Settings > Mods also lists changes written in C (`port/mods/builtin.c`): faster travel, cheaper inns,
heavier blows, a strong Quickstart party, a rich start. New ones are a hook on a game function
(`hook_install`) plus an entry in the `MODS` table.
