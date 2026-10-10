# darklands-re

This is reverse-engineering groundwork for **Darklands** (MicroProse, 1992, MS-DOS), version 483.07 (the GOG release). The aim is a matching decompilation as a verifiable baseline, then moddable reimplementations of each game module.

This repository contains **no game files** and no code generated from them. Everything under `build/` is regenerated from your own copy of `DARKLAND.EXE`.

## What works today

* **Matching decompilation, in progress.** 370 original source files (about 8,400 of the ~10,100 functions, 880 KB of code) are C that Microsoft C 6.00A compiles back to the original bytes; `tools/build.py` rebuilds a **byte-identical** `DARKLAND.EXE` (MD5 `0d51cc9458a6724e1bd34a9fe0d81e7f`) from that C plus reassembled code for the rest. Nearly all town/event card scripts (s044–s135) are done, plus parts of the engine (strategic rules s029 1367, saints s039 165c, card engine pieces, combat sub-modules, character creation screens). Engine drafts that are complete but not yet byte-exact live in `src/wip/` (overland map, inventory, camping, alchemy, loot, card engine, strategic core, battlefields).
* **Mod builds.** `tools/build.py --mod --overlay <mod dir>` builds a game in which edited C modules may grow or shrink: the linker re-lays out the overlay tree (`omflink.relayout`), and every code reference is symbolic. Tested in DOSBox: moving every card overlay and growing modules still plays correctly; the complete `src/wip` engine drafts run too (the overland map from C). See `docs/CARDS.md` and the example in `mods/rich_start/`.
* **Readable card scripts.** `include/darklands/card.h` names ~280 engine variables and rule functions (`next_card`, `opt_fn`, `money_set`, `advance_hours`, `practice_skill` ...); all card files use them and still match. `docs/CARDS.md` is the modder's guide.
* **Readable engine.** `include/darklands/game.h` names ~600 more engine globals, far data and functions (`combatants`, `floor_img`, `mem_alloc`, `gfx_copy_rect`, `combat_main` ...); `tools/gamenames.py` applies both headers to `src/c/root`, `src/c/s000..s043` and `src/wip` (93 files converted, fmatch results unchanged). `docs/ENGINE.md` maps every engine module for modders.
* **Headless play-testing.** `tools/play.py` runs the game in DOSBox on a virtual display with scripted mouse/keys and screenshots (`QUICKSTART` macro starts a game).
* **Toolchain.** MSC 6.00A runs under emu2 (`tools/msc.py`); `tools/dec16.py` decompiles MSC 6 code to C automatically (about 80% of card-script functions match straight away); `tools/fmatch.py` checks a C file function by function; `docs/PORTING.md` and `docs/PORTING_NOTES.md` collect the compiler idioms learned.
* **Whole-program map, data formats, Ghidra export** as before: `docs/MODULE_MAP.md`, `formats/` (round-trip readers/writers for every shipped data file), `tools/run_ghidra.sh`.

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

The data-format work builds on vvendigo's Darklands tools and on documentation by Merle and Joel "Quadko" McIntyre. The overall method follows neuviemeporte's F-15 Strike Eagle II reconstruction and mzretools.
