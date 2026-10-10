# Roadmap

There are two goals: a **matching decompilation** as a verifiable baseline, then **moddable** C for every module.

## Where things stand (2026-10-10)

| milestone | status |
|---|---|
| Byte-identical rebuild from reassembled code + C | done (`tools/build.py`) |
| Matching C | 384 files, ~8,500 of ~10,100 functions (84%); all but ~12 card scripts; much of the engine |
| Whole game from C ("functional" build) | ~9,500 functions (94%) from C incl. drafts in `src/wip/`; play-tested against the original (`docs/QA.md`) |
| Mods that change code size | done: overlay relayout, data-section move, code-segment strings (`--mod`, `--overlay`, `--functional`) |
| Readable names | `include/darklands/card.h`, `game.h`; guides `docs/CARDS.md`, `docs/ENGINE.md` |
| Example mods | `mods/` (travel speed, inn price, combat damage, party stats, start money) |

Remaining work:
* **Last matches.** The non-matching functions are mostly code-generation differences between the game's MSC 6.00 and our 6.00A (block placement, register choice, far memset). An original MSC 6.00 would likely resolve many at once.
* **Not C by nature:** C run-time library (could be matched by linking MSC's own library objects), RTLink/Plus manager, PKWARE DCL, graphics-driver glue (root 047c), a few hand-written routines.
* **Card scripts as data:** card text already lives in `msgfiles/*.msg`; card logic is now readable C with a documented API. A higher-level card description format that generates this C is a possible next step.

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
