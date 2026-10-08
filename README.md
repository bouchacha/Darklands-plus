# darklands-re

This is reverse-engineering groundwork for **Darklands** (MicroProse, 1992, MS-DOS), version 483.07 (the GOG release). The aim is a matching decompilation as a verifiable baseline, then moddable reimplementations of each game module.

This repository contains **no game files** and no code generated from them. Everything under `build/` is regenerated from your own copy of `DARKLAND.EXE`.

## What works today

* **Byte-identical rebuild.** `tools/build.py` disassembles the 1.6 MB executable into 489 reassemblable source modules (one per original segment, plus split-out functions), assembles them with JWasm, and links them with a purpose-built RTLink/Plus-compatible linker. The result is a byte-identical `DARKLAND.EXE` (MD5 `0d51cc9458a6724e1bd34a9fe0d81e7f`). All relocation tables, the overlay section table and the MZ header are regenerated, not copied.
* **Whole-program map.** It covers 9,697 functions across the resident root and 136 overlay sections, classified as compiled C (about 94% of the code by size), hand-written assembly, the C run-time library and the overlay manager. Callers, callees and referenced strings are in `build/analysis/functions.json`.
* **Module map.** It records which overlay is combat, battlefield generation, the overland map, inventory, barter, alchemy, saints, character creation and save/load, the card engine, and the 92 town/event card-script overlays (`docs/MODULE_MAP.md`).
* **Toolchain identified.** Microsoft C 6.0, medium model, `/G2`, linked with RTLink/Plus (`docs/TOOLCHAIN.md`).
* **Data files.** There are readers for every shipped data format and writers for nearly all of them, verified by byte-identical round trips on all 1,648 files and archive entries (`formats/`).
* **Ghidra pseudo-C** as a porting reference (`tools/run_ghidra.sh`). It is done for all non-card-script code: 1,704 functions.

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
