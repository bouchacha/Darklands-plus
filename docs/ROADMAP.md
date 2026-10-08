# Roadmap

There are two goals: a **matching decompilation** as a verifiable baseline, then **moddable reimplementations** of each module.

## Where things stand

| milestone | status |
|---|---|
| Executable format fully decoded (MZ + RTLink/Plus overlay tree, vectors) | done; repack is byte-identical |
| Whole-program disassembly with exact segment frames, overlay calls and switch tables resolved | done; 501,542 instructions, 9,697 functions |
| Reassemblable source plus a replacement linker that rebuild the EXE **byte-identically** | done (`tools/build.py`) |
| Any function can be split into its own module, still byte-identical | done (`symbols/split.txt`) |
| Toolchain identified (MSC 6.0 medium model `/G2`, RTLink/Plus) | done (`docs/TOOLCHAIN.md`) |
| C run-time library and entry points named | done (`symbols/names.txt`) |
| Game-module map (combat, battlefield, overland, inventory, barter, alchemy, saints, character creation, card engine, card scripts) | done (`docs/MODULE_MAP.md`) |
| Data files: readers/writers with byte-identical round trips on every shipped file | done (`formats/`) |
| Ghidra pseudo-C as a reference for C porting | done for the resident code and s000–s043: 1,704 of 1,792 functions (95%). The card scripts are skipped in favour of `tools/card_templates.py`; `tools/run_ghidra.sh` generates them too (several hours) |

## Phase 1: matching C, module by module

**Needs: a copy of Microsoft C 6.00 or 6.00A.** CL.EXE, C1/C2/C3 and MLIBCE.LIB, run headless under DOSBox the way the F-15 SE2 project does. Nothing else is missing.

1. Pin the flags. Compile three or four known functions with candidates (`/AM /G2 /Gs /Ox`, `/Os`, `/Zp1` and so on) until the bytes match.
2. Name the graphics-driver calls. Root segment 047c holds the same self-patching far-jump slot table that F-15 SE2 uses to call `MGRAPHIC.EXE`/`MISC.EXE`, and that project's `slot.h` documents 84 graphics slots by name. Mapping those names onto 047c names much of one of the most-called segments in the game (2,440 call sites).
3. Port leaf modules first: the root utilities, then the strategic screens (s031 inventory, s032 barter, s035 cache, s040 loot, s033 alchemy, s034 saints). These are small and self-contained, and their data formats are already documented.
4. Port a whole original module (one region / `.asm` file) at a time. That way `push cs` / `call near` same-module calls match.
5. Verify with `tools/build.py`. A matched function leaves the EXE byte-identical.

**The card scripts are mostly templates.** s044–s135 hold 8,130 of the 9,697 functions (about 900 KB, 65% of all code). `tools/card_templates.py` groups the 7,905 script functions by code template, where only the constants differ. It finds 1,739 templates, and the 10 most common cover 56% of the functions. The top 50 cover 72% and the top 200 cover 80%. The most common template, 1,763 functions, just copies a help string ("No risk.", "Enter Combat") and returns -2. Two other templates are 793 copies of identical code.

This code was almost certainly produced by a card-script generator. So for most of it, matching C can be generated mechanically: write each template once, then extract the constants from every instance. That turns the biggest part of the job into a tooling task.

## Phase 2: modding seams

Each ported module becomes a seam with documented inputs and outputs:

* **Card scripts become data.** The same constant extraction that drives the generated C can emit a readable card-script format: card id → options, skill checks, outcomes, text. The engine (s043/s039) then interprets that data, and new or changed encounters stop needing code. This is where most of the game's content lives.
* **Combat (s014 + children), battlefields (s004–s013), overland (s030), inventory (s031)** are ordinary C modules. Once ported, they can be changed directly in C.
* **Rules hard-coded in the EXE** (from the data-format work): saint and potion effects, weapon and armour numbers, encounter tables, shop stock, character-creation numbers. Move these into data tables as their modules are ported.

## Phase 3: beyond DOS (optional)

With the C in hand, the medium-model DOS specifics (far pointers, the MGRAPHIC/MISC drivers, RTLink overlays) can be wrapped behind a thin platform layer for a native port. The F-15 SE2 project followed the same path (reconstruction, then a separate porting project).

## Known gaps

* Offsets into DGROUP stay numeric in the generated assembly. That's fine for matching. Before C code can add globals or string literals, DGROUP needs labels; `tools/funcdb.py` already finds 1,500+ referenced strings to seed them.
* About 1.9% of instructions are emitted as raw bytes because JWasm picks a different encoding. That's harmless for matching, but less readable.
* The linker keeps the original layout. Code that grows or shrinks, as non-matching mods will, needs the overlay tree re-laid out. That's a known, contained extension of `tools/omflink.py`, because all code references are already symbolic.
* No runtime testing yet. The next tooling step is DOSBox, built from source, for smoke tests of modified builds.
