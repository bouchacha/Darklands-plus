<img width="619" height="1118" alt="0-questlog" src="https://github.com/user-attachments/assets/a72197df-9f75-4ef0-a614-e4a336ce6573" />
<img width="1456" height="819" alt="9-saiints" src="https://github.com/user-attachments/assets/477a8867-698e-47ae-a860-eec2076f8cbf" />
<img width="1456" height="819" alt="8-alchemy" src="https://github.com/user-attachments/assets/172be814-4dee-4d70-87d6-5f3e4962deb7" />
<img width="1600" height="900" alt="7-character-creation" src="https://github.com/user-attachments/assets/9aa8b892-331e-42b2-8ef6-a5ad71042313" />
<img width="1600" height="900" alt="6-world-map-find" src="https://github.com/user-attachments/assets/39355c90-db70-40a1-8618-32b10f00ac7b" />
<img width="1600" height="900" alt="5-battle-log" src="https://github.com/user-attachments/assets/939f66cb-e63a-4157-921b-f792fbe429fc" />
<img width="1600" height="900" alt="4-character-sheet" src="https://github.com/user-attachments/assets/5b952690-99c6-4ae9-87e2-b36530a174fb" />
<img width="1600" height="900" alt="3-shop" src="https://github.com/user-attachments/assets/79d2de07-0f3d-41a1-81ed-e49185c12597" />
<img width="1600" height="900" alt="2-city-go-to" src="https://github.com/user-attachments/assets/9ac5e161-fc1a-4b9a-8317-e58c9e4d652b" />
<img width="1600" height="900" alt="1-cards-and-margins" src="https://github.com/user-attachments/assets/f14d37a2-c53b-44bc-a8c6-714ad1171413" />


This is reverse-engineering groundwork for **Darklands** (MicroProse, 1992, MS-DOS), version 483.07 (the GOG release). The aim is a matching decompilation as a verifiable baseline, then moddable reimplementations of each game module.

> **Status: finished and handed over.** The author is not a programmer and will not maintain this
> project or answer bug reports (the issue tracker is switched off). It is free for anyone to
> take over: fork it, fix it, improve it, and publish your own releases. The code was written
> entirely by an AI (Anthropic's Claude), working from the author's direction and play-testing;
> `CONTRIBUTING.md` and `docs/` are written so that you, or an AI assistant of your own, can
> carry on. See `LICENSE` for what is and is not covered.

This repository contains **no game files** and no code generated from them. Everything under `build/` is regenerated from your own copy of `DARKLAND.EXE`.

## Playing: Darklands+

**Darklands+** runs Darklands natively on Windows, without DOSBox. It adds optional improvements: most of the game's screens rebuilt for the mouse and modern screens (cards, shops, the character sheet, loot, the store room, inn stays, alchemy, saints, save and load ...; docs/NATIVE_SCREENS.md), the party's best member making the card tests the game puts to the leader, one pack for the whole party, quests with titles, deadlines and pictures on the maps, chances of success and the reasons for greyed options on cards, crisp text, a world map with auto-travel, a battle log, tooltips for items, saints and formulae, fixes for the original's bugs, the Roland MT-32 built in (with your own ROM files), game-controller support, a bestiary and mod folders (a sample battle-sound mod comes with it). Every addition can be switched off, and "Play the original game" turns them all off at once. What is new since 1.0: `CHANGELOG.md`.

* **Download** the Windows zip from this repository's Releases, unpack it anywhere and start `darklands.exe`. It finds a GOG or Steam installation of Darklands by itself, or asks for the folder once; an installation under Program Files (GOG Galaxy's) works as it is, with the saves in Windows' VirtualStore. You need your own copy of Darklands 483.07 (GOG or Steam).
* **On the first start** Darklands+ spends a minute or two translating the game program from your `DARKLAND.EXE`. The download contains nothing from the game itself.
* Players' notes: `port/tools/release_readme.txt`. Features and keys: `port/README.md`. Making mods: `docs/MODDING.md`. Working on it: `CONTRIBUTING.md`. Taking parts into another engine: `docs/MERGING.md`.

## What works today

* **Native Darklands ("Darklands+").** `port/` recompiles the game to C and runs it natively on Linux and Windows (no DOSBox), with a modern layer on top that can be switched off entirely: crisp text, a zoomable world map with auto-travel, helpers (chances of success, saints, "Go to" in cities, shop and battle info), quick saves and autosaves, fixes for original bugs, and mod folders for data, card texts, menu texts, weapon numbers, items, art and sounds. See `port/README.md` and `docs/MODDING.md`.

* **Matching decompilation.** 382 original source files — about 8,500 of the game's ~10,100 functions (84%), 900 KB of code — are C that Microsoft C 6.00A compiles back to the original bytes. `tools/build.py` rebuilds a **byte-identical** `DARKLAND.EXE` (MD5 `0d51cc9458a6724e1bd34a9fe0d81e7f`) from that C plus reassembled code for the rest. All but ~13 card scripts are done, and large parts of the engine (strategic rules, saints, character creation, combat sub-modules, PAN player, heap/EMS/XMS code).
* **The whole game from C (functional build).** `tools/build.py <EXE> --skip-analysis --mod --functional` adds the complete-but-not-yet-byte-exact drafts in `src/wip/` (overland map, inventory, barter, camping, alchemy, loot, card engine, strategic core, tactical combat engine, battlefield generators, start-up, game shell...): about 9,500 functions (94%) come from C. It is play-tested in DOSBox against the original (`docs/QA.md`): city, inns, market, overland travel, encounters, battles and loot, character creation, save/load, alchemy, saints, menus. What is left in assembly is the C run-time library, the RTLink overlay manager, the PKWARE compression library, the graphics-driver glue and a few hand-written routines.
* **Mods.** `tools/build.py --mod --overlay <mod dir>` (optionally with `--functional`) builds a game in which edited modules may grow or shrink: the linker re-lays out the overlay tree and can move the data section. New strings go into the code segment (`static char _based(_segname("_CODE")) s[]`). Example mods in `mods/`: faster overland travel, half-price inn, double damage against enemies, strong starting party, richer start.
* **Readable source.** `include/darklands/card.h` (~280 names) and `include/darklands/game.h` (~600 names) give engine variables and rule functions readable names (`next_card`, `money_set`, `combatants`, `path_find`...); all card scripts and most engine files use them and still compile to the same bytes. Guides: `docs/CARDS.md` (card scripts), `docs/ENGINE.md` (engine modules, data layouts, original-game bugs).
* **Headless play-testing.** `tools/play.py` runs the game in DOSBox on a virtual display with scripted mouse/keys and screenshots; `tools/playtests/` has scenario scripts and a lock-step runner that plays the original and a test build side by side.
* **Toolchain.** MSC 6.00A under emu2 (`tools/msc.py`), an MSC-6 decompiler (`tools/dec16.py`), per-function match checking (`tools/fmatch.py`); `docs/PORTING.md` and `docs/PORTING_NOTES.md` collect the compiler idioms learned. The remaining non-matching functions are mostly code-generation differences between the game's MSC 6.00 and the 6.00A available to us.
* **Data formats, module map, Ghidra export** as before: `formats/`, `docs/MODULE_MAP.md`, `tools/run_ghidra.sh`.

## Quick start

```
pip install capstone
# build JWasm: git clone https://github.com/Baron-von-Riedesel/JWasm && make -f GccUnix.mak
JWASM=/path/to/jwasm python3 -I tools/build.py /path/to/DARKLAND/DARKLAND.EXE
python3 -I tools/funcdb.py  /path/to/DARKLAND.EXE build/analysis/analysis.pickle
python3 -I tools/modules.py /path/to/DARKLAND.EXE build/analysis
python3 -I formats/tests/test_roundtrip.py          # DARKLANDS_DIR=/path/to/DARKLAND
```

## Layout

| path | contents |
|---|---|
| `tools/rtlink.py` | RTLink/Plus executable parser and repacker |
| `tools/analyze.py` | recursive-descent analysis (segments, overlay vectors, switch tables) |
| `tools/emit_asm.py` | reassemblable MASM/JWasm source generator |
| `tools/omflink.py` | OMF linker that writes the RTLink layout |
| `tools/build.py` | end-to-end rebuild and comparison |
| `tools/funcdb.py`, `tools/modules.py` | function database, module map |
| `tools/ghidra_prep.py`, `tools/ghidra/`, `tools/run_ghidra.sh` | Ghidra headless pseudo-C export |
| `symbols/names.txt` | function names (applied everywhere) |
| `symbols/split.txt` | functions emitted as standalone modules, ready for C replacement |
| `formats/` | data-file library, tests and `docs/FORMATS.md` |
| `docs/` | toolchain, rebuild pipeline, module map, roadmap |

## Credits

The data-format work builds on vvendigo's Darklands tools and on documentation by Merle and Joel "Quadko" McIntyre. The overall method follows neuviemeporte's F-15 Strike Eagle II reconstruction and mzretools. Darklands+ uses Dear ImGui, SDL2, Nuked OPL3, Munt's libmt32emu, stb and the fonts listed in `port/README.md`; the sample battle sounds are made from 0 A.D.'s recordings (Wildfire Games, CC BY-SA 3.0).
