# Modding the native port

The native port (`port/`) reads mods from folders, so the game's own files are never changed and
every mod can be switched off again. Everything here is done from the game's F12 menu; no tools
are needed.

## Mod folders

A mod is a folder in the game folder's `mods` folder:

```
<game>/mods/My mod/
    mod.txt                 first line: the name shown in F12 > Mods; the rest: a description
    DARKLAND.LST            any game file, replaced whole (items, saints, cities, enemies ...)
    PICS/CASTLE1.PIC        one picture
    MSGFILES/$MAINS01.TXT   one card file as text (or .MSG); the other cards stay as they are
    A00C.CAT/<entry>        one file inside a .CAT catalogue (also BC, LCASTLE)
    rules.ini               weapon numbers (below)
    art/  sounds/           replacement pictures (PNG) and sounds (WAV/OGG), as in the game folder
    music/                  recordings that replace the game's pieces of music (below)
```

Tick the mod in F12 > Mods. Several mods can be on at once; where two change the same file the
folder that comes later in the alphabet wins (name them `10 base`, `20 tweaks` ... to set the
order). Only reading is redirected: saves and settings stay in the game folder.

When the game folder cannot be written (Windows: under Program Files, where GOG Galaxy installs
games), the `mods`, `art` and `sounds` folders are in `%LOCALAPPDATA%\Darklands+` instead, and
the game's saves go to Windows' VirtualStore copy of the game folder; F12 > About shows where.

Card texts, pictures, sounds and weapon numbers change at once; files the game reads when it
starts (the item list, saints, cities ...) need a restart.

Implementation: `port/mods/datamods.c` (redirect of DOS opens for reading, catalogue merging into
`mods/.cache/`), `port/ui/rules.cpp` (rules.ini).

## Card texts

F12 > Mods > "Export the card texts" writes all 419 card files of MSGFILES as text into
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

## Replacement art and sounds

`art/NAME.png` replaces the picture `PICS/NAME.PIC` (or a sprite of that name) at any resolution,
drawn at window resolution; F12 > Display can export all original pictures as templates
and show the name of the picture under the mouse. `sounds/NAME.wav` (or `.ogg`, `NAME2.wav` ...
for variations) replaces a battle sound; the names are listed in F12 > Display. Mods can
carry both folders; a mod's files win over the game folder's.

### Making a whole art pack

`port/tools/artpack.py ORIGINALS OUT` turns the exported originals (F12 > Display > "Export
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
the release (off until ticked in F12 > Mods): 50 takes of the eleven battle sounds, cut from
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
The pieces are the same for every sound device. F12 > Display > Sound lists the numbers that
are known (a church 8, the inn 9 and 10, the market 11, the guilds and craftsmen 12, the city
square 14, the university 15, the fortress 17, the dungeon 22, before a city 27 and 28, a
village 41, the ending 45, the opening 47 ...; 8 to 47 in all) and names each piece as it starts, so a pack can be made by listening.

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

F12 > Mods also lists changes written in C (`port/mods/builtin.c`): faster travel, cheaper inns,
heavier blows, a strong Quickstart party, a rich start. New ones are a hook on a game function
(`hook_install`) plus an entry in the `MODS` table.
