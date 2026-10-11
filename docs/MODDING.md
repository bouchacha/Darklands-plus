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
```

Tick the mod in F12 > Mods. Several mods can be on at once; where two change the same file the
folder that comes later in the alphabet wins (name them `10 base`, `20 tweaks` ... to set the
order). Only reading is redirected: saves and settings stay in the game folder.

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

F12 > Item records > Weapons lists the battle numbers of every weapon; "Write these numbers into a
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

## Items

F12 > Item records > Items lists every item of DARKLAND.LST with its name, short name, weight,
quality, rarity and value, all editable; "Save the list into this mod folder" writes the whole
list as the mod's `DARKLAND.LST`. The game reads the list when it starts.

## Replacement art and sounds

`art/NAME.png` replaces the picture `PICS/NAME.PIC` (or a sprite of that name) at any resolution,
drawn at window resolution; F12 > Display & sound can export all original pictures as templates
and show the name of the picture under the mouse. `sounds/NAME.wav` (or `.ogg`, `NAME2.wav` ...
for variations) replaces a battle sound; the names are listed in F12 > Display & sound. Mods can
carry both folders; a mod's files win over the game folder's.

## Built-in changes

F12 > Mods also lists changes written in C (`port/mods/builtin.c`): faster travel, cheaper inns,
heavier blows, a strong Quickstart party, a rich start. New ones are a hook on a game function
(`hook_install`) plus an entry in the `MODS` table.
