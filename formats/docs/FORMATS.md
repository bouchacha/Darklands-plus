# Darklands data formats

Reference for the `dlformats` package (`formats/dlformats/`). All formats
were checked against the GOG copy of Darklands 483.07. Every writer was
tested by re-creating every shipped file of its type **byte for byte**
(`tests/test_roundtrip.py`).

**Credits.** This builds on community work: Merle's XML format docs
(wallace.net/darklands), Joel "Quadko" McIntyre's PIC notes, Olemars'
IMC/decompression code, and vvendigo's readers and updated docs
(github.com/vvendigo/Darklands). Corrections to those docs are marked
**[correction]** and new findings **[new]**.

**Conventions.**
* All integers are little-endian unless marked `be`. `char[N]` is a NUL-padded
  string in the Darklands character set (see [Character set](#character-set)).
* Bytes after the NUL in a fixed string are kept as `<field>__pad`. Some files
  (CTY, saves) contain leftover editor memory there.
* Fields named `unknown_XX` (XX = hex offset) are not understood. They are kept
  verbatim.
* The "confidence" column says how sure we are of a field's **meaning**.
  Offsets and sizes are always confirmed by the exact round-trip.
  * `structural`: the meaning follows from the structure (strings, counts, offsets).
  * `cross-ref`: confirmed by consistency with another file (for example, the
    item type/code pairs in ENM match LST).
  * `doc`: the name comes from Merle/vvendigo and was not checked independently.
  * blank: unknown.
* Game modules: overland, town, inventory, combat, chargen, dialog, graphics.
* The record tables below are generated from the `Spec` definitions in the
  modules (`Spec.markdown()`), so code and docs share one source.

## Usage

```python
import sys; sys.path.insert(0, "formats")        # or install the package
from dlformats import cat, lst, msg, pic, lzss
lst_data = lst.read("DARKLAND/DARKLAND.LST")      # -> dataclass of dicts
msgs = cat.read("DARKLAND/MSGFILES")              # CAT archive
cards = msg.parse(msgs.get("$CITYE00.MSG")).cards
p = pic.read("DARKLAND/PICS/XALTAR.PIC"); pic.to_png(p, "xaltar.png")
assert lst.build(lst_data) == open("DARKLAND/DARKLAND.LST", "rb").read()
```

* Tests: `cd formats && python3 -I tests/test_roundtrip.py`. Set
  `DARKLANDS_DIR` to point at the game directory.
* Dump everything to JSON and PNG:
  `python3 -I tools/dump_all.py [OUTDIR] [--game DIR]`. The default output is
  `/home/claude/darklands-re/build/data_dump/`.
* Dependencies: the Python 3 standard library, plus Pillow for PNG
  import/export only.

## Summary

| format | files | read | write | round-trip verified | coverage notes |
|---|---|---|---|---|---|
| CAT archive (`*.CAT`, MSGFILES, BC, LCASTLE) | 11 | yes | yes | 11/11 | Fully understood. The timestamp is a DOS date/time **[correction]** |
| LZ compression (in IMC/IMAPS/BC/LCASTLE/PAN) | 585 streams | yes | yes | 585/585 | Encoder reconstructed exactly **[new]** |
| PIC image | 607 (547 PICS + 60 EINFO) | yes (to PNG) | yes (from PNG) | 607/607 | LZW+RLE encoders exact. Palette padding understood |
| DARKLAND.LST items/saints/formulae | 1 | yes | yes | 1/1 | Some flag bits and 2 relic words unknown |
| DARKLAND.ALC formulae | 1 | yes | yes | 1/1 | Complete |
| DARKLAND.SNT saint texts | 1 | yes | yes | 1/1 | Complete (text only, no saint effects) |
| DARKLAND.DSC city descriptions | 1 | yes | yes | 1/1 | Complete. The header byte is not the record count |
| DARKLAND.ENM enemies | 1 | yes | yes | 1/1 | Equipment fields decoded **[new]**. About 40 bytes per type still unknown |
| ENEMYPAL.DAT | 1 | yes | yes | 1/1 | 4 trailing bytes per record unknown |
| DARKLAND.LOC / LOCS.TMP | 2 | yes | yes | 2/2 | Offsets corrected. 6 small fields unknown |
| DARKLAND.CTY | 1 | yes | yes | 1/1 | Building bits re-derived **[correction]**. 5 fields unknown |
| MSG card files (in MSGFILES) | 419 (3756 cards) | yes | yes | 419/419 | Complete as presentation. Logic is in the EXE |
| DARKLAND.MSG | 1 | yes | yes | 1/1 | Complete |
| DARKLAND.MAP | 1 | yes | yes | 1/1 | Complete. The tile-variant rule is in the EXE |
| Save game (SAVES/DEFAULT, *.SAV) | 1 | yes | yes | 1/1 (DEFAULT only) | 166 header and 44 per-character bytes unknown. Events partly known |
| CHARACTR.TMP / CACHE.TMP | 2 | yes | yes | 2/2 | Sub-blocks of the save layout |
| FONTS.FNT / FONTS.UTL | 2 | yes (to PNG) | yes | 2/2 | Complete. Extra line-spacing rows found **[new]** |
| IMC battle sprites | 333 (18472 frames) | yes (to PNG) | yes | 333/333 | Frame encoding complete. Some header fields unknown |
| BATTLEGR.IMG / COMMONSP.IMG | 2 | yes | yes | 2/2 | Identified as sprite banks **[new]**. 208-byte preamble unknown |
| IMAPS.CAT tactical maps | 143 | partial | yes | 143/143 | 40x40 grid found **[new]**. Cell bits and tail unknown |
| BC / LCASTLE entries | 94 | decompress only | yes | 94/94 | Payload formats not decoded |
| *.PAN animations | 15 | decompress only | yes | 15/15 | Identified as LZ streams **[new]**. Frames not decoded |
| TACANIM.DB, HITSEQ.DTA, COMNCLRS.DAT, BKGNDPAL.DAT, DARKLAND.FAM, CFILE.DAT, DARKLAND.CFG, BANNER.DAT | 8 | yes | yes | 8/8 | Record layouts known. Most meanings unknown |
| DARKLAND.EXE string tables | 1 | yes (read-only) | no | n/a | 10 tables at fixed offsets, checked |
| *.DGT digitised sound | 3 | n/a | n/a | n/a | Headerless 8-bit unsigned PCM. Rate not stored |
| TERRAIN.FIL, LC_*.FIL/.ATV, LEVEL0.ENM, CONFIG.DRK, DARKLAND.CCH | | no | no | no | Raw. See [Not handled](#not-handled) |

In total, 1649 files and catalog entries are re-created byte-exactly: 600
top-level files and 1049 catalog entries. Run
`python3 -I tests/test_roundtrip.py` from `formats/`. It takes about 2 minutes,
mostly in the pure-Python LZ and LZW encoders.

## What is hard-coded in the executable

The data files hold content: names, texts, numbers and graphics. The
following **behaviour and tables are not in any data file**, so they must
live in `DARKLAND.EXE` and its overlays. These are the main targets for the
code side of the project.

1. **Dialog/event flow.** The MSG cards hold presentation only: text, option
   labels, margins and option *kinds*. Everything else is in the EXE:
   * which card file and card is shown for a location `menu` id (LOC
     `menu`, save `curr_menu`/`prev_menu`). A table of 366 card-file names sits
     at EXE offset 0x196502, and 58 of the 419 card files are not named in
     it, so they are reached some other way.
   * what each option does, and the skill checks, rolls and outcomes.
   * when the `0x01 ... 0x13` conditional text blocks are shown (15 cards).
   * the values of the 91 distinct `$Variables`. Only 65 of them appear in
     the EXE's variable-name table at 0x188AF4. `$VarPhrase1..7`,
     `$DieRoll`, `$McGuffin` and others are resolved elsewhere.
2. **Saint effects.** SNT is flavour text only. Saint bonuses, durations,
   divine-favour costs and prerequisites are not in the data.
3. **Potion effects.** ALC holds the recipes, difficulty and risk. What each
   potion does is not in the data.
4. **Item mechanics.** LST has no damage, penetration or armour values. ENM
   uses item *types* 34–63 and 85–94 that no LST item has ("natural"
   weapons and armour of animals and monsters, for example type 0x2C for
   the wolf). Their stats must be EXE tables.
5. **Enemy tables.** The counts 71 and 82 are not stored. The image groups M08
   (Undead), M12 (DemonKt) and M99 (Baphomet) have no sprites and no
   TACANIM entry, so the EXE must remap them. Baphomet's palette index points
   one past ENEMYPAL. Encounter tables (who appears where and when) are not
   in the data.
6. **Overland rules.** The map tile-variant (neighbour) rule, terrain
   movement costs and passability, travel time, random encounter rates.
7. **Town economy.** Shop inventories and prices are generated from CTY
   quality bytes and the save's `city_contents_seed`. The algorithm and item
   pools are in the EXE.
8. **Character generation.** Name lists, occupations and family backgrounds
   are EXE string tables (extracted by `exe_tables.py`). The attribute and
   skill changes per occupation and age are presumably numeric tables next
   to them.
9. **Quests and events.** The meaning of the save `event` fields (type,
   state) and quest generation.
10. **Combat.** Tactical map semantics (IMAPS cell bits, the BC tile
    components), AI, hit resolution (HITSEQ.DTA is data, but its meaning is
    unknown), and the meaning of the IMC header motion bytes.
11. **Presentation.** Which palette goes with which screen (BKGNDPAL,
    COMNCLRS target ranges) and the PAN animation player.

## Character set

Darklands text uses a German ISO-646-like variant
(`util.dl_to_unicode`):

| byte | glyph | evidence |
|---|---|---|
| 0x1F | ä | text ("J\x1fger", "D\x1fnemark") and font glyph |
| 0x7B `{` | ö | text ("G{rlitz", "L{we") and font glyph |
| 0x7C `\|` | ü | text ("L\|beck", "N\|rnberg") and font glyph |
| 0x5B `[` / 0x5C `\` / 0x5D `]` | Ä / Ö / Ü | font glyph shape only |
| 0x5F `_` | ß | font glyph shape only |

Card text also uses control bytes. See [MSG cards](#msg-card-files).

---

## CAT archives

*Module: all (container). `dlformats.cat`. Files: A00C, C00C, E00C, F01C,
F60C, M00C (battle sprites), EINFO (enemy info PICs), IMAPS (tactical maps),
MSGFILES (dialog cards), BC (battle tile components), LCASTLE (final castle).*

| off | type | name | meaning |
|---|---|---|---|
| 0x00 | u16 | count | number of entries |
| 0x02 | entry[count] | index | 24 bytes each |
| ... | | data | entry data, contiguous, in index order |

Entry: `char name[12]` (8.3, NUL-padded), `u32 dos_datetime`, `u32 length`,
`u32 offset`.

* **[correction]** The timestamp is a DOS packed date/time: low word is the
  time, high word is the date. All 1050 entries decode to valid dates from
  1991-01-24 to 1993-02-25. Earlier docs guessed "seconds since 1978/1979".
* LCASTLE contains a zero-length entry named `LCASTLE`.
* Verified: index and data are contiguous in all 11 archives. `build()`
  recreates all of them exactly. `replace()`/`from_directory()` support mods.

## LZ compression

*`dlformats.lzss`. Used by every IMC, IMAPS, BC and LCASTLE entry and every
`*.PAN` file.*

vvendigo called this "DRLE". It is actually an LZSS variant whose bitstream
has the same layout as LZEXE 0.91's. Control bits come from LE 16-bit words,
LSB first. The next control word is read as soon as the 16th bit has been
consumed, so control words are interleaved with the data bytes.

| bits | data | meaning |
|---|---|---|
| `1` | byte | literal |
| `0 0 a b` | byte B | copy `(a<<1\|b)+2` (2..5) bytes from distance `256-B` (1..256) |
| `0 1` | word W | `x = (W>>11)<<8 \| (W&0xFF)`, distance `0x2000-x`. If `n=(W>>8)&7` is non-zero, copy `n+2` bytes. Otherwise read byte C: `0` = end, `1` = segment marker (no output), else copy `C+1` bytes |

**[new]** The original encoder was reconstructed. It reproduces all 585
shipped streams exactly (570 catalog entries and 15 PAN files):
* Greedy longest match, at most 253 bytes, within 7939 bytes (an 8 KiB ring
  buffer minus the 253-byte look-ahead). On a tie the nearest match wins.
* Matches are measured against padded input: past the end of the data the
  bytes are `data[n+k-8192]` (stale ring contents) or 0. Lengths are then
  clipped to what remains. This only affects the last tokens.
* A length-2 match is used only if its distance is ≤ 255. Otherwise a literal
  is emitted. Lengths 3–5 at distance ≤ 255 use the short form.
* When the output has advanced ≥ 0xA000 bytes since the last segment marker,
  another marker is emitted, if more data follows.
* The end marker is `W=0xF000, C=0`. Three PAN files carry 13–15 bytes after
  it. These are kept as `Pan.trailer`.

## PIC images

*Module: graphics (all screens). `dlformats.pic`. Files: `PICS/*.PIC` (547
images, plus PICSHOW.EXE/.OVL, a DOS viewer, not images) and the 60 EINFO.CAT
entries.*

A PIC file is a sequence of chunks `{u16 tag; u16 length; data}`:

| tag | name | content |
|---|---|---|
| 0x304D `M0` | palette | `u8 first, u8 last`, `(last-first+1)` × RGB (6-bit). The length is padded to even: 12 files have 1 pad byte **[new]** |
| 0x3058 `X0` | image | `u16 width, u16 height, u8 max_bits` (always 11), then LZW(RLE(pixels)) |

* LZW: LSB-first codes. The width is 9 bits and grows after 256, then 512,
  then 1024 codes. When it would exceed `max_bits`, it resets to 9 and the
  dictionary resets. Entry 256 is unused and new entries start at 257.
  Padding at the end is zero bits.
* RLE: `0x90 n` means the previous byte appears `n` times in total.
  `0x90 0x00` is a literal 0x90. The encoder makes runs of 4..255 across the
  whole image (crossing scanlines) and never run-encodes 0x90.
* Both encoders reproduce all 607 images exactly. `from_png()` builds new
  PICs from indexed PNGs.
* Palettes: 366 files define colours 16..255, 6 define 0..255, 4 define
  1..255, 8 define 17..255, and 1 defines 128..159. 222 files have no palette
  and use whatever palette the screen has loaded. The PNG export falls back
  to the VGA default palette for those, so their colours are wrong.
* `CHARPALT.PIC` has a 0x0 image (one stray pixel) and is effectively a
  palette.
* MAPICONS.PIC and MAPICON2.PIC are the overland tile sheets. The artists
  wrote terrain labels next to each row (see `map.TERRAIN_LABELS`).

## DARKLAND.LST

*Modules: inventory, town (shops), combat. `dlformats.lst`.*

| off | type | name |
|---|---|---|
| 0x00 | u8 | num_item_slots (200) |
| 0x01 | u8 | num_saints (136) |
| 0x02 | u8 | num_formulae (66) |
| 0x03 | item_definition[200] | items. 172 are defined, codes 172..199 are all-zero |
| ... | cstr[136] ×2 | saint full names ("St.…"), then short names ("S.…") |
| ... | cstr[66] ×2 | formula full names, then short names |

`item_definition`, 46 (0x2E) bytes:

| off | type | name | meaning | confidence |
|---|---|---|---|---|
| 0x00 | char[20] | name | full name | structural |
| 0x14 | char[10] | short_name | short name | structural |
| 0x1E | u16 | type | item type id (used with the code in equipment slots/ENM) | cross-ref |
| 0x20 | u8[5] | flags | 5 flag bytes, see FLAG_BITS / docs (bit names from Merle, re-checked against item lists) | cross-ref |
| 0x25 | u8 | weight | weight | doc |
| 0x26 | u8 | quality | default quality (25 normal, 99 quest items) | doc |
| 0x27 | u8 | rarity | rarity 0..12 | doc |
| 0x28 | u16 | unknown_28 | non-zero only for relics (6..80) |  |
| 0x2A | u16 | unknown_2a | non-zero only for relics and 'Residency Permit' |  |
| 0x2C | u16 | value | base value | doc |

* The index of an item is its **item code**. The `type` word is a second
  id. Equipment slots in saves and all ENM equipment fields use *both*,
  and they always agree with this table (verified).
* Flag bytes, as `FLAG_BITS`. These were re-checked against the item lists:
  * 0x20 b0..7 = edged, impact, polearm, flail, thrown, bow, metal armour, shield.
  * 0x21 b2..6 = component, potion, relic, horse, quest_1. b0 = torch,
    candle, rope, iron spike, lantern, harp, flute. b1 = clock, grappling
    hook, lockpick, harp, flute.
  * 0x22 b0,1,2,4,5,7 = lockpicks, light, arrow, quarrel, ball, quest_2.
  * 0x23 b0 = throwable potion, b2 = non-metal armour, b3 = missile weapon.
    **[correction]** b4 is the group Merle called "mine chest?" (15 items)
    and **b5 = music** (Harp, Flute). Merle placed these one bit higher.
  * 0x24: exactly one of b0, b1, b7 is set for every item except
    non-clothing armour and the Superb/Fast Horse. b7 = clothing, quest
    items and relics. b0 = one-handed weapons, shields, tools, some potions.
    b1 = two-handed weapons, bows, missile weapons, ammunition, components,
    other potions. The meaning is unknown.
  * Never set: 0x21 b7, 0x22 b3/b6, 0x23 b1/b6/b7, 0x24 b2..b6.
* Unknown: `unknown_28` and `unknown_2a`, which are non-zero only for relics.

## DARKLAND.ALC

*Modules: inventory (alchemy screen). `dlformats.alc`.*

`u8 count (66)` followed by 66 × 104-byte `formula_definition`, in LST
formula order.

`formula_definition`, 104 (0x68) bytes:

| off | type | name | meaning | confidence |
|---|---|---|---|---|
| 0x00 | char[80] | description | description text | structural |
| 0x50 | u16 | mystic_number | base difficulty | doc |
| 0x52 | u16 | risk_factor | 0 low, 1 medium, 2 high | doc |
| 0x54 | ingredient[5] | ingredients | ingredients (usually, not always, ascending code), zero-filled | cross-ref |

`ingredient`: `u16 quantity` (0 = unused), `u16 item_code`.

* Verified: every ingredient is an LST item with `is_component`.
  **[correction]** Ingredients are *not* always in ascending code order
  (15 of 66 are not).
* The formulae come in 22 groups of 3 per potion, each by a different named
  author. Saves track 3 bits per potion. The mapping of formula `3k+j` to
  bit `j` is assumed, not verified.

## DARKLAND.SNT

*Modules: inventory/dialog (saints screen). `dlformats.snt`.*

`u8 count (136)` followed by 136 × `char[360]` descriptions in LST saint
order. The file is complete, and it holds **only text**.

## DARKLAND.DSC

*Modules: dialog (`$PlaceDesc`). `dlformats.dsc`.*

`u8 header` (0x5E = 94) followed by 92 × `char[80]` descriptions in CTY
order. The header byte does not match the 92 records. The reader takes the
count from the file size and keeps the byte.

## DARKLAND.ENM

*Module: combat, encounters. `dlformats.enm`.*

71 × 204-byte `enemy_type` at offset 0, then 82 × 24-byte `enemy` at 0x3894.
Neither count is stored in the file.

`enemy_type`, 204 (0xCC) bytes:

| off | type | name | meaning | confidence |
|---|---|---|---|---|
| 0x00 | char[4] | image_group | sprite group, e.g. 'E00' (E00C.CAT / M00C.CAT, TACANIM.DB) | cross-ref |
| 0x04 | char[10] | type_name | internal name, e.g. 'Sergeant1' | structural |
| 0x0E | u8 | num_variants | number of variants for the first record of a group, 0xFF for the others | structural |
| 0x0F | u8 | palette_count | number of ENEMYPAL.DAT palettes usable | cross-ref |
| 0x10 | u8 | unknown_10 | 1..3 |  |
| 0x11 | u8 | palette_start | first ENEMYPAL.DAT palette | cross-ref |
| 0x12 | u16 | unknown_12 | 0..2 |  |
| 0x14 | attribute_set | attrs | attributes | doc |
| 0x1B | skill_set | skills | skills | doc |
| 0x2E | u8 | unknown_2e | 8..20 |  |
| 0x2F | u8 | unknown_2f | always 0 |  |
| 0x30 | u8 | unknown_30 | 0, 0x01, 0x20, 0x2B, 0x40 (skeletons/gnomes/vulcan/hellhound/dragons) |  |
| 0x31 | u8 | unknown_31 | 0..2 |  |
| 0x32 | u8[66] | potion_kit | 22 triplets; non-zero only for some types (Merle: potion odds/quality) |  |
| 0x74 | u8[30] | component_kit | 10 triplets (Merle: component odds?) |  |
| 0x92 | u8 | vital_armor_type | LST item type of vital armour (or natural-armour type) | cross-ref |
| 0x93 | u8 | limb_armor_type | LST item type of limb armour | cross-ref |
| 0x94 | u8 | vital_armor_code | LST item code of vital armour, 0xFF none | cross-ref |
| 0x95 | u8 | limb_armor_code | LST item code of limb armour, 0xFF none | cross-ref |
| 0x96 | u8 | armor_quality | armour quality (effective AC for unarmoured creatures) | doc |
| 0x97 | u8 | shield_type | LST item type of shield, 0xFF none | cross-ref |
| 0x98 | u8 | shield_code | LST item code of shield, 0xFF none | cross-ref |
| 0x99 | u8 | shield_quality | shield quality | doc |
| 0x9A | u8 | missile_type | LST type of missile weapon, 0xFF none | cross-ref |
| 0x9B | u8 | missile_code | LST code of missile weapon | cross-ref |
| 0x9C | u8 | missile_quality | quality of missile weapon |  |
| 0x9D | u8 | ammo_type | LST type of ammunition | cross-ref |
| 0x9E | u8 | ammo_code | LST code of ammunition | cross-ref |
| 0x9F | u8 | ammo_count | amount of ammunition (6..12 for sergeants) |  |
| 0xA0 | u8[6] | weapon_types | candidate melee weapon LST types (0xFF none) | cross-ref |
| 0xA6 | u8[6] | weapon_codes | matching LST codes (0xFF none) | cross-ref |
| 0xAC | u8 | weapon_quality | weapon quality | doc |
| 0xAD | u8[12] | unknown_ad | mostly 0xFE/0xFF, occasional small values |  |
| 0xB9 | u8[19] | unknown_b9 | mostly zero; small values / 0xFFFF words |  |

`attribute_set` (7 × u8): end, str, agl, per, int, chr, df.
`skill_set` (19 × u8): wEdg, wImp, wFll, wPol, wThr, wBow, wMsl, alch, relg,
virt, spkC, spkL, r_w, heal, artf, stlh, strw, ride, wdws. For enemies, all
melee skills are equal, all missile skills are equal, and the rest are 20.

`enemy`, 24 (0x18) bytes:

| off | type | name | meaning | confidence |
|---|---|---|---|---|
| 0x00 | u16 | enemy_type | index into enemy_types | structural |
| 0x02 | char[12] | name | display name | structural |
| 0x0E | u8[8] | unknown_0e | always zero |  |
| 0x16 | u16 | unknown_16 | 0,1,3,7,0x3F,0x4F,0x81; same for all enemies of a type |  |

* **[correction]** Skills start at 0x1B, not 0x1A. Merle's 0x92..0xAC
  "unknown + vital_type + ..." block was re-mapped. The fields are
  (LST **type**, LST **code**) pairs for vital armour, limb armour, shield,
  missile weapon and ammunition, followed by 6 weapon types and 6 matching
  weapon codes. Every pair agrees with LST except Skeleton1/2 (natural type
  48 paired with code 8, Battle Axe). **[new]**
* `palette_start`/`palette_count` index ENEMYPAL.DAT. Baphomet points one
  past the end.
* Unknown: 0x10, 0x12, 0x2E–0x31, the 0xAD and 0xB9 blocks, and
  `enemy.unknown_16`.
* The "potion kit" (0x32, 22 triplets, one per potion kind) is non-zero
  only for EvilAlch, Monk, Dwarf2, Dwrf Over and DemonLcst. The "component
  kit" (0x74, 10 triplets) is non-zero for 24 types (alchemists, witches,
  monks, templars, demons, dragons, Baphomet…). The triplets look like
  (chance?, ±quantity?, quality), for example `0a f8 19`. They are
  presumably loot or usable items. Their semantics are in the EXE.

## ENEMYPAL.DAT

*Module: combat (sprite recolouring). `dlformats.enemypal`.*

71 × 53 bytes:

`enemy_palette`, 53 (0x35) bytes:

| off | type | name | meaning | confidence |
|---|---|---|---|---|
| 0x00 | u8 | start | palette byte offset (/3 = first colour index) | doc |
| 0x01 | rgb[16] | colors | 16 colours, 6-bit | doc |
| 0x31 | u8[4] | unknown_31 | unknown |  |

## DARKLAND.LOC (and LOCS.TMP)

*Module: overland. `dlformats.loc`.*

`u16 count (414)` followed by `location[count]`. The same record is used in
saves and in LOCS.TMP, a runtime copy that differs by 1 byte. Records 0..91
are the cities in CTY order.

`location`, 58 (0x3A) bytes:

| off | type | name | meaning | confidence |
|---|---|---|---|---|
| 0x00 | u16 | icon | map icon / location kind (see LOCATION_ICONS, from Merle) | doc |
| 0x02 | u16 | unknown_02 | 0 for cities; 8..14 otherwise |  |
| 0x04 | u16 | x | map x (0..326) | structural |
| 0x06 | u16 | y | map y (0..930; one record has 931 = off-map, Merle: Marienburg is patched at new game) | structural |
| 0x08 | u16 | unknown_08 | 5 for cities, 1/9/10 otherwise (raubritter strength?) |  |
| 0x0A | u16 | unknown_0a | 0..5, 99 for pagan altars |  |
| 0x0C | u16 | menu | card/menu id shown on entering (EXE table) | doc |
| 0x0E | u16 | unknown_0e | 0x62, 0x92 for raubritter castles |  |
| 0x10 | u8 | unknown_10 | always 0xFF in DARKLAND.LOC |  |
| 0x11 | u8 | city_size | 3..8 for cities, 1 otherwise | cross-ref |
| 0x12 | s16 | local_rep | local reputation (0 in DARKLAND.LOC, set in saves) | doc |
| 0x14 | u8 | unknown_14 | 0 = active; 0x04 destroyed, 0x20/0x24 seen in saves (Merle) |  |
| 0x15 | u8[3] | unknown_15 | always 0x19 0x19 0x19 |  |
| 0x18 | u16 | inn_cache_idx | 0xFFFF, or index into the save's cache_offsets | doc |
| 0x1A | u16 | unknown_1a | always 0 |  |
| 0x1C | u16 | unknown_1c | 0, except 0xC0 for one location |  |
| 0x1E | u8[8] | unknown_1e | always zero |  |
| 0x26 | char[20] | name | name (DL charset) | structural |

* **[correction]** The name is at 0x26, not 0x25. Merle's overlapping words at
  0x19/0x1B are really `u16 0x1A` (always 0) and `u16 0x1C` (0, or 0xC0 for
  one record).
* Verified: city records match CTY coordinates and size, except Strassburg
  (x 49 here, 50 in CTY). One location has y = 931, off the 931-row map.
* `menu` is a card/menu id interpreted by the EXE: cities 4, villages 133,
  castles 242, and so on.
* Unknown: 0x02, 0x08, 0x0A, 0x0E, 0x10 and 0x14 (Merle gives guesses; see
  the field notes).

## DARKLAND.CTY

*Modules: overland, town. `dlformats.cty`.*

`u8 count (92)` followed by 92 × 622-byte `city`.

`city`, 622 (0x26E) bytes:

| off | type | name | meaning | confidence |
|---|---|---|---|---|
| 0x00 | char[32] | short_name | short name | structural |
| 0x20 | char[32] | full_name | full name | structural |
| 0x40 | city_data | data | see city_data |  |
| 0x6E | char[32] | leader_name | title of the city leader | structural |
| 0x8E | char[32] | ruler_name | overlord | structural |
| 0xAE | char[32] | unknown_famous | always 'Famous Place' | structural |
| 0xCE | char[32] | polit_name | town square | structural |
| 0xEE | char[32] | town_hall_name |  | structural |
| 0x10E | char[32] | fortress_name |  | structural |
| 0x12E | char[32] | cathedral_name |  | structural |
| 0x14E | char[32] | church_name |  | structural |
| 0x16E | char[32] | market_name |  | structural |
| 0x18E | char[32] | unknown_munz_name | Munzenplatz (6 cities) | structural |
| 0x1AE | char[32] | slum_name |  | structural |
| 0x1CE | char[32] | unknown_zeughaus_name | Zeughaus / -turm / -tor | structural |
| 0x1EE | char[32] | pawnshop_name | Leihhaus | structural |
| 0x20E | char[32] | kloster_name |  | structural |
| 0x22E | char[32] | inn_name |  | structural |
| 0x24E | char[32] | university_name |  | structural |

`city_data` (at 0x40, 46 bytes):

`city_data`, 46 (0x2E) bytes:

| off | type | name | meaning | confidence |
|---|---|---|---|---|
| 0x00 | u16 | city_size | 3..8 | cross-ref |
| 0x02 | u16 | entry_x | map coordinates of the city | cross-ref |
| 0x04 | u16 | entry_y |  | cross-ref |
| 0x06 | u16 | exit_x | party position after leaving | doc |
| 0x08 | u16 | exit_y |  | doc |
| 0x0A | u16[4] | dock_destinations | city indices reachable by ship, 0xFFFF none | cross-ref |
| 0x12 | u16 | coast | 0xFFFF inland, 0/1 (Merle: side of river; tidal) | doc |
| 0x14 | u16 | unknown_14 | always 4 |  |
| 0x16 | u16 | pseudo_ordinal | 0..91 with one gap and one duplicate |  |
| 0x18 | u16 | city_type | 0 free city, 1 ruled, 2 capital (selects $CITYE00 card text, Merle) | doc |
| 0x1A | u16 | unknown_1a | 0..3 |  |
| 0x1C | u16 | unknown_1c | always 0 |  |
| 0x1E | u16 | contents | building bitmask, see CONTENT_BITS | cross-ref |
| 0x20 | u16 | unknown_20 | always 0 |  |
| 0x22 | u8 | qual_blacksmith | shop quality, 0 = no shop (relative, Merle) | doc |
| 0x23 | u8 | qual_merchant |  | doc |
| 0x24 | u8 | qual_swordsmith |  | doc |
| 0x25 | u8 | qual_armorer |  | doc |
| 0x26 | u8 | qual_unknown_26 | unknown shop quality |  |
| 0x27 | u8 | qual_bowyer |  | doc |
| 0x28 | u8 | qual_tinker |  | doc |
| 0x29 | u8 | qual_unknown_29 | unknown shop quality |  |
| 0x2A | u8 | qual_clothing |  | doc |
| 0x2B | u8 | unknown_2b | always 0 |  |
| 0x2C | u8 | unknown_2c | 1..255 |  |
| 0x2D | u8 | unknown_2d | 0 or 1 |  |

* Every string field carries editor memory garbage after the NUL. It is kept
  in `__pad`. Empty building names are a single space.
* **[correction]** `contents` bits were re-derived by correlating each bit
  with its name string over all 92 cities: b1 town hall (92/92), b2
  fortress (91), b4 cathedral (92), b5 "Zeughaus" name (89), b6 slums (91),
  b7 kloster (92), b8 university (91), b9 pawnshop (91), b10 "Munzenplatz"
  name (92), b11 docks (91/92, matches the dock list). b0 is set in 91 cities
  and b3 in all of them, meanings unknown. b12–15 are never set. Merle's
  order (kloster = b0, ...) is wrong.
* Unknown: `qual_unknown_26`, `qual_unknown_29`, 0x1A, 0x2C, 0x2D, and the
  "Famous Place" string.

## MSG card files

*Module: dialog (all menus and events). `dlformats.msg`. 419 `$*.MSG` files
in MSGFILES, 3756 cards.*

`u8 num_cards`, then for each card a 5-byte header and NUL-terminated text:

`card_header`, 5 (0x5) bytes:

| off | type | name | meaning | confidence |
|---|---|---|---|---|
| 0x00 | u8 | text_y | top text margin | doc |
| 0x01 | u8 | text_x | left text margin | doc |
| 0x02 | u8 | unknown_2 | 0 for nearly all cards |  |
| 0x03 | u8 | text_max_x | right text edge | doc |
| 0x04 | u8 | unknown_4 | 0 for nearly all cards |  |

Text mark-up:

| byte | meaning |
|---|---|
| 0x0A | line break |
| 0x14 | paragraph spacing (usually after 0x0A) |
| 0x15 | normal option (3411 uses) |
| 0x16 | option that opens the saint list (313) |
| 0x10 | option that opens a list of potions or formulae (148) |
| 0x06 | option that starts a battle (101) |
| 0x1D | separator between the "..." lead-in and the option text |
| 0x01 … 0x13 | conditional text block shown or hidden by the EXE (15 cards) **[new]** |
| 0x19 | only in "<0x19>ou decide" ($MONAS00/01), apparently a glyph used as 'Y' |
| 0x1F, `{`, `\|` | ä, ö, ü |
| `$Name` | substitution variable (91 distinct) |

* Unused option slots hold dummy text ("0", "1", …).
* `unknown_2`/`unknown_4` are non-zero in 47 cards, mostly $MCGUF07.MSG,
  which has 76 cards with empty text. These may be parameters rather than
  layout.
* `analyse_card()` splits a card into body, options and variables.
  `readable()` renders the mark-up.
* **Not in the data**: everything listed under
  [hard-coded](#what-is-hard-coded-in-the-executable), item 1. vvendigo's
  conclusion that the dialog logic is hidden in the executable is confirmed.
  The card files carry no ids, jumps, conditions or effects.

## DARKLAND.MSG

*Module: dialog/system. `dlformats.msg.parse_darkland_msg`.*

`u8 count (10)` followed by 10 × `char[400]` texts (error-message title,
raubritter cairn text, the wasteland text, …). This is a different format
from the card files.

## DARKLAND.MAP

*Module: overland. `dlformats.map`.*

| off | type | name |
|---|---|---|
| 0x00 | u16be | width (327). **Big-endian** |
| 0x02 | u16be | height (931) |
| 0x04 | u32[height] | absolute row offsets (rows are contiguous) |
| ... | | RLE rows: byte `ccc b rrrr` = repeat 1..7, bank (0 MAPICONS / 1 MAPICON2), row |

* A cell value is `bank<<4 | row` (0..31). The labels the artists painted
  on the tile sheets: 0 plains, 1 ocean, 2 major river, 3 minor river, 4
  tidal marsh, 5 marsh, 6 Geest, 7 Gst1, 8/9 farm, 10/11 F/W, 12/13 LtW,
  14–17 forest, 18–21 rock, 22/23 alpine, 25 ford, 27 bridge, 28 castle, 29
  city. Rows 24, 26, 30 and 31 are unlabelled.
* The greedy encoder reproduces the file exactly.
* The tile *column* (0..15 neighbour variant) is computed by the EXE.
  Merle's rule is reportedly wrong (vvendigo). Locations are in LOC, not
  here.

## Save games

*Modules: all. `dlformats.sav`. Only `SAVES/DEFAULT` (the new-game template)
ships, so the layout is verified on a single file.*

| off | content |
|---|---|
| 0x000 | header (0xEF bytes) |
| 0x0EF | `u16 num_curr_characters`, `u16 num_characters` |
| 0x0F3 | `s16 party_char_indices[5]` (-1 = empty) |
| 0x0FD | `char party_images[5][4]` ("F60", …; empty = "0\0" "0\0") |
| 0x111 | `person_colors[5]` (8 RGB triplets each) |
| 0x189 | `character[num_characters]` (0x22A each) |
| ... | `u16 num_events`, `event[n]` (0x30 each) |
| ... | `u16 num_locations`, `location[n]` (LOC record). DEFAULT has 405, while LOC has 414 |
| ... | `u8 max_cache_slot (0x63)`, `u8 num_caches`, `u16 cache_offsets[98]` |
| ... | cache data (kept raw; `parse_caches()` decodes on a best-effort basis) |

Header:

`save_header`, 239 (0xEF) bytes:

| off | type | name | meaning | confidence |
|---|---|---|---|---|
| 0x00 | char[12] | curr_location_name | name of current location | structural |
| 0x0C | u8[9] | unknown_0c |  |  |
| 0x15 | char[23] | label | save game label | structural |
| 0x2C | u8[55] | unknown_2c | Merle: 18 zero bytes + 37 unknown; not zero in DEFAULT |  |
| 0x63 | u8 | unknown_63 | Merle: constant 0x19; 0x64 in DEFAULT |  |
| 0x64 | u16 | city_contents_seed | seed for shop/saint/formula availability (Merle) | doc |
| 0x66 | u16 | unknown_66 | 7 |  |
| 0x68 | date_ymdh | date | current date: year, month(0-based), day, hour | structural |
| 0x70 | money | money | cash | doc |
| 0x76 | u8[4] | unknown_76 |  |  |
| 0x7A | s16 | reputation | global reputation | doc |
| 0x7C | s16 | curr_location | index into locations, -1 = wilderness | doc |
| 0x7E | u16 | x | map x | doc |
| 0x80 | u16 | y | map y | doc |
| 0x82 | u16 | curr_menu | current card/menu id | doc |
| 0x84 | u8[6] | unknown_84 |  |  |
| 0x8A | u16 | prev_menu | previous card/menu id | doc |
| 0x8C | u16 | bank_notes | bank notes (florins) | doc |
| 0x8E | u8[4] | unknown_8e |  |  |
| 0x92 | u16 | philosopher_stone | philosopher's stone quality | doc |
| 0x94 | u8[7] | unknown_94 |  |  |
| 0x9B | u8[5] | party_order | marching order (0-based) | doc |
| 0xA0 | u8 | unknown_a0 |  |  |
| 0xA1 | u8 | party_leader | leader index (0-based) | doc |
| 0xA2 | u8[3] | unknown_a2 |  |  |
| 0xA5 | u8[74] | unknown_a5 | zero in DEFAULT |  |

Character:

`character`, 554 (0x22A) bytes:

| off | type | name | meaning | confidence |
|---|---|---|---|---|
| 0x00 | u8[18] | unknown_00 |  |  |
| 0x12 | u16 | age | age | doc |
| 0x14 | u8 | unknown_14 |  |  |
| 0x15 | u8 | shield | heraldic shield letter 'A'..'O' (PICS/SHIELD?.PIC) | doc |
| 0x16 | u8[12] | unknown_16 |  |  |
| 0x22 | u8 | equip_missile_type | LST type of equipped missile weapon (0xFF none) | cross-ref |
| 0x23 | u8[2] | unknown_23 |  |  |
| 0x25 | char[25] | full_name | full name | structural |
| 0x3E | char[11] | short_name | nickname | structural |
| 0x49 | u8[2] | unknown_49 |  |  |
| 0x4B | u8 | equip_vital_type | LST type of vital armour | cross-ref |
| 0x4C | u8 | equip_leg_type | LST type of leg armour | cross-ref |
| 0x4D | u8[2] | unknown_4d |  |  |
| 0x4F | u8 | equip_vital_q | quality | doc |
| 0x50 | u8 | equip_leg_q | quality | doc |
| 0x51 | u8 | equip_weapon_type | LST type of weapon | cross-ref |
| 0x52 | u8[6] | unknown_52 |  |  |
| 0x58 | u8 | equip_weapon_q | quality | doc |
| 0x59 | u8 | unknown_59 |  |  |
| 0x5A | u8 | equip_missile_q | quality | doc |
| 0x5B | u8 | equip_shield_q | quality | doc |
| 0x5C | u8 | equip_shield_type | LST type of shield (0xFF none) | doc |
| 0x5D | attribute_set | curr_attrs | current attributes | doc |
| 0x64 | attribute_set | max_attrs | maximum attributes | doc |
| 0x6B | skill_set | skills | skills | doc |
| 0x7E | u16 | num_items | items in use | structural |
| 0x80 | u8[20] | saints_known | bit per saint (LST order) | doc |
| 0x94 | u8[22] | formulae_known | byte per potion, 3 bits (Merle: q45/q35/q25) | doc |
| 0xAA | item[64] | items | inventory (unused slots zero) | structural |

`item` (6 bytes):

`item`, 6 (0x6) bytes:

| off | type | name | meaning | confidence |
|---|---|---|---|---|
| 0x00 | u16 | code | LST item code | cross-ref |
| 0x02 | u8 | type | LST item type (== LST[code].type) | cross-ref |
| 0x03 | u8 | quality | quality | doc |
| 0x04 | u8 | quantity | quantity | doc |
| 0x05 | u8 | weight | weight | doc |

Event:

`event`, 48 (0x30) bytes:

| off | type | name | meaning | confidence |
|---|---|---|---|---|
| 0x00 | s16 | unknown_00 | RB quests: giver id (Merle) |  |
| 0x02 | date | create_date | creation date | doc |
| 0x0A | date | date_0a | == create_date (Merle) |  |
| 0x12 | date | expire_date | expiry date | doc |
| 0x1A | s16 | unknown_1a | quests: location given / -2 |  |
| 0x1C | s16 | unknown_1c | quests: target location |  |
| 0x1E | s16 | unknown_1e | quests: return location |  |
| 0x20 | s16 | unknown_20 |  |  |
| 0x22 | s16 | unknown_22 | state? (RB quest 8/0x24/7, Merle) |  |
| 0x24 | s16 | unknown_24 |  |  |
| 0x26 | s16 | unknown_26 |  |  |
| 0x28 | s16 | unknown_28 |  |  |
| 0x2A | s16 | unknown_2a |  |  |
| 0x2C | s16 | unknown_2c |  |  |
| 0x2E | s16 | unknown_2e | quest item code (Merle) |  |

`date` = hour, day, month (0-based), year. The header date is stored as
year, month, day, hour.

* **[correction]** Event words are at even offsets 0x1A..0x2E; Merle has 0x21/0x23/0x25. Merle's
  "constant zero" block at 0x2C and "constant 0x19" byte at 0x63 do not hold
  in DEFAULT.
* Verified: the four DEFAULT characters' item `type` bytes match LST, and the
  unused item slots are zero.
* **CHARACTR.TMP** = `u16 num_characters` + the party block (0xF3..0x110) +
  characters, with no colours. Its characters equal DEFAULT's.
  **CACHE.TMP** = the cache section.
* Unknown: 166 header bytes, 44 bytes per character, and most event
  fields. The `.BSV` battle save is not present.

## FONTS.FNT / FONTS.UTL

*Module: graphics. `dlformats.fnt`.*

`u16 count`, `u16 offset[count]` (each points at the font's bitmap). Each
font is stored as:

| rel. to bitmap | type | name |
|---|---|---|
| -8-n | u8[n] | glyph widths (n = last-first+1) |
| -8 | u8 | first_char |
| -7 | u8 | last_char |
| -6 | u8 | bytes_per_row |
| -5 | u8 | unknown (0) |
| -4 | u8 | height |
| -3 | u8 | char_spacing (1) |
| -2 | u8 | line_spacing (1, or 0 for UTL font 3) |
| -1 | u8 | unknown (0) |
| 0 | u8[(height+line_spacing)·n·bpr] | bitmap, row-major across all glyphs, MSB = left |

* **[new]** The bitmap has `height + line_spacing` rows. The extra rows are
  blank. Without them, the fonts appear to be separated by unexplained gaps.
  FONTS.UTL has one alignment byte before font 3.
* FONTS.FNT: font 0 covers chars 0–127 (0–31 are UI symbols); fonts 1–2
  cover 31–127. FONTS.UTL has the same three fonts (a few glyphs differ)
  plus a small fourth font.

## IMC battle sprites

*Module: combat. `dlformats.imc`. 333 files in A00C/C00C/E00C/F01C/F60C/M00C.
The name is `<group><WK|CB|DY><weapon>.IMC` (walk / combat / dying).*

Each file is an LZ stream. Decompressed layout:

| off | type | name |
|---|---|---|
| 0x00 | u16[12] | unknown |
| 0x18 | u16 | motion_flag: 0xFFFF in DY files, otherwise 0..4 |
| 0x1A | u8[20] | (non-DY only) motion bytes. Small signed values per direction? |
| B+0 | u16 | height (B = 0x1A for DY, 0x2E otherwise) |
| B+2 | u16[8] | widths per direction (often, not always, frame widths) |
| B+18 | u16[8] | unknown |
| B+34 | u16 | frame_count (per direction) |
| B+36 | u16 | data_length |
| B+38 | u16[8·frame_count] | frame offsets in 16-byte paragraphs from the data start. Unsorted, may repeat |
| ... | frames | `u8 w, u8 h`, then per row `u8 count, u8 skip, u8 pixels[count]` (0 = transparent) |

* Verified on all 333 files: `data_length` is exact, all 18472 frames
  decode inside their slots, and every frame height equals `height`.
* Frames are not stored in index order, and the padding between them is
  sometimes non-zero. The reader therefore keeps the raw frame area for
  exactness. `with_frames()` builds a fresh, valid layout for mods.
* Unknown: the 12 leading words, the motion bytes, the second width array,
  and how the 8 frame groups map to directions.

## BATTLEGR.IMG / COMMONSP.IMG

*Module: combat. `dlformats.imc.parse_img`.* **[new]** These are
uncompressed sprite banks: `u32 data_length`, `u16 frame_offset[n]` (n
inferred from the file size, in paragraphs), then frames in the IMC encoding.
BATTLEGR has 200 frames and COMMONSP has 13. The 208 bytes before
BATTLEGR's first frame are unknown.

## IMAPS.CAT tactical maps

*Module: combat (city, mine, wilderness and dungeon battle maps).
`dlformats.imaps`.* Each of the 143 entries is an LZ stream that
decompresses to exactly 13308 bytes. **[new]** The first 6400 bytes are a
40×40 grid of u32 cells, where 0x80000000 marks void and outside cells
(rendering shows city blocks and wilderness borders). The cell bit layout
and the remaining 6908 bytes are unknown and kept raw. LCASTLE's `LC_*`
maps decompress to 13310 bytes and look similar.

## BC and LCASTLE entries

*Module: combat.* BC holds `*.FLC/FFC/FRC/NWC/WWC/NFC/WFC`, presumably
battle-map tile components (floor, walls…). LCASTLE holds `LC_*` maps,
`*.FIL` (196-byte terrain/tile lists like TERRAIN.FIL) and `*.ATV` (32
bytes). All 94 are LZ streams and re-compress exactly. Their payloads are
not decoded.

## *.PAN animations

*Module: graphics (intro, endings, death, credits). `dlformats.pan`.*
**[new]** Each file is one LZ stream, sometimes followed by a few trailing
bytes. The data decompresses to 106–700 KB, starting with `u16 0x0A5A`
(magic) and `u16` 55..358 (a frame or chunk count?). The frame encoding is
not decoded.

## Small files

*`dlformats.misc` (all round-trip exactly):*

| file | layout | notes |
|---|---|---|
| TACANIM.DB | 39 × {char group[4]; u16 ×3; u8 ×4} | one record per sprite group. Meaning unknown (vvendigo saw no effect editing the last 4 bytes) |
| HITSEQ.DTA | 56 × 15 bytes | small values 0..7 and some 0x8x. Probably combat sequences |
| COMNCLRS.DAT | 72 RGB (6-bit) | palette fragment. Target range unknown |
| BKGNDPAL.DAT | 11 × 71 RGB (6-bit) | **[new]** 11 background palette variants. Magenta entries look unused |
| DARKLAND.FAM | 5 × {u16, u16, u16, char name[4][25]} | possibly a hall of fame |
| BANNER.DAT | u16 offsets[20] + 20 cstrings | loader banner and memory-check messages |
| CFILE.DAT | 4 × {u16, u16, u8, u8} | unknown |
| DARKLAND.CFG | char[13] sound driver, char[13] graphics driver, char[64] path | install configuration |

Field tables:

`tacanim`, 14 (0xE) bytes:

| off | type | name | meaning | confidence |
|---|---|---|---|---|
| 0x00 | char[4] | group | image group | cross-ref |
| 0x04 | u16[3] | unknown_04 | unknown |  |
| 0x0A | u8[4] | unknown_0a | unknown (editing had no visible effect, vvendigo) |  |

`fam_record`, 106 (0x6A) bytes:

| off | type | name | meaning | confidence |
|---|---|---|---|---|
| 0x00 | u16 | unknown_00 |  |  |
| 0x02 | u16 | unknown_02 |  |  |
| 0x04 | u16 | unknown_04 |  |  |
| 0x06 | name25[4] | names | four names |  |

`darkland_cfg`, 90 (0x5A) bytes:

| off | type | name | meaning | confidence |
|---|---|---|---|---|
| 0x00 | char[13] | sound_driver | e.g. 'nsound.dl' | structural |
| 0x0D | char[13] | graphics_driver | e.g. 'mgraphic.exe' | structural |
| 0x1A | char[64] | path | install path | structural |

## *.DGT digitised sound

OPENDARK, ENDDARK1 and ENDDARK2 are raw, headerless 8-bit unsigned mono PCM
(mean 127.5, full 0–255 range). The sample rate is not in the file.
`misc.dgt_to_wav(data, path, rate)` requires the caller to supply it.

## DARKLAND.EXE string tables

`dlformats.exe_tables` extracts 10 NUL-separated string tables from the
483.07 EXE (file size checked, plus the first string of each table):
attributes, skills, occupations, family backgrounds, card variables, male
and female first names, surnames, background picture names, and card file
names. Offsets are from vvendigo's `reader_exe.py`. Binary tables are left to
the code side.

## Not handled

* **Executables and drivers:** DARKLAND.EXE, DKED.EXE (probably a data
  editor; unexamined), DKQUE.EXE, MGRAPHIC.EXE, INSTALL.EXE, MPSCOPY.EXE, XTRACT.EXE,
  QUERY.EXE, MISC.EXE, BOOTDISK.EXE, DARKPOP.COM, PICS/PICSHOW.EXE/.OVL, and
  the sound drivers `*.DB/*.DC/*.DLB/*.DLC`, which are MZ executables. These
  belong to the code side.
* **Runtime files:** LEVEL0.ENM (291 bytes: a count byte, then one instance
  record of the current battle's enemies, containing an ENM-like
  attribute/skill block), DK.TMP, MEM.LOG, HEAP.FNG (512 KiB of zeros),
  CONFIG.DRK (4 × u16 options), DARKLAND.CCH (106 bytes, unknown).
  TAC.TXT is a developer debug dump of tactical parameters ("Tac params…",
  "Enemy Activation Record"). It hints at the field names of the battle
  setup structure.
* **TERRAIN.FIL / LC_*.FIL:** 196 bytes each, holding names ("mine",
  "lastcast") and byte lists with editor garbage. The record stride is
  unclear.
