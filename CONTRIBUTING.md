# Working on Darklands+

This project is meant to be carried on by the Darklands community. This page is the map: where
things are, how to build and test them, and how a change usually goes. The details are in the
documents it points to.

## The rule that matters most

**Nothing from the game goes into the repository or a release.** No game files, no code made
from `DARKLAND.EXE` (everything under `build/`), no `darklands_game.dll`, no pictures or sounds
exported from the game. The program translates the player's own `DARKLAND.EXE` on its first
start. Packs made from the game's pictures (`port/tools/artpack.py`, `aiart.py`) are for their
maker's own use. `port/tools/package.py` refuses to pack game-derived files, and `.gitignore`
keeps `build/` out.

## Two projects in one repository

* **The decompilation** (`src/`, `tools/`, `include/`, `docs/REBUILD.md`, `docs/PORTING*.md`):
  C that Microsoft C 6.00A compiles back to the original bytes, and a pipeline that rebuilds a
  byte-identical `DARKLAND.EXE`. Work here is matching functions; `docs/ROADMAP.md` says what is
  left.
* **Darklands+** (`port/`): the game recompiled to C and run natively, with the improvements on
  top. Most player-facing work happens here. `port/README.md` explains how it works.

## Darklands+ in one paragraph

`port/recomp` translates each function of the game into C that runs on an emulated x86 state
(`runtime/cpu.h`); `runtime/` provides DOS, the BIOS, the sound cards (Nuked OPL3, Munt's MT-32,
an MPU-401), SDL2 for the window, and **hooks**: any game function can be wrapped or replaced
(`hook_install(section, seg, off, fn)`, `hook_call_original()`; arguments are read from the
stack, `rw(SS, SP + 4)` and on). `mods/` holds the changes written in C against hooks (bug
fixes, helpers, crisp text, replacement art and sounds, mod folders). `ui/` is the Dear ImGui
layer: the F12 menu, tooltips, margins, maps, journal. Addresses in comments are the game's own
(`s043 150b:1142` = section 43, segment 150b, offset 1142, as the analysis names them); the
generated code and the decompiled sources (`src/`, `include/darklands/*.h` for readable names)
say what a function does.

## Building and running

```
python3 -I tools/analyze.py /path/to/DARKLAND/DARKLAND.EXE build/analysis   # once; pip install capstone
GAME=/path/to/DARKLAND sh port/tools/gen.sh                                   # the game's code as C (build/port_gen)
make -C port -j8                                                              # Linux: build/port/darklands
build/port/darklands /path/to/DARKLAND
make -C port SPLIT=1 TARGET=win64 -j8                                         # the Windows program without game code
```

Windows builds use mingw-w64 and SDL2 in `build/sdl2-win64`; `make ASAN=1` builds with
AddressSanitizer. A release is made on GitHub by `.github/workflows/release.yml`
(`docs/RELEASING.md`).

## Testing a change

All tests copy the game folder; it is never changed.

* `python3 -I port/tools/smoke.py --game DIR [--exe EXE] [--wine]`: start, battle, save and load.
  Run it before every release, also under Wine with the Windows build.
* `sh port/tools/batch.sh OUTDIR`: the standard play tests with screenshots.
* `python3 -I port/tools/monkey.py NAME MINUTES --game DIR [--x11]`: random play as a soak test;
  it reports crashes and sessions that stop changing. Run a sanitizer build with it now and then.
* New encounters: `port/tools/enccheck.c` checks scripts without the game (build line at its
  top); `python3 -I port/tools/encfuzz.py build/enccheck` feeds it mutated scripts under the
  sanitizers. Run it after changing the script language in `port/mods/encounters.c`.
* `python3 -I port/tools/nctl.py start NAME --x11 --game DIR` and `nctl.py do NAME "steps"`:
  drive a session by hand from a script and take screenshots (`wshot`); the test commands
  (`ucmd ucard:`, `uload:`, `uhover:x,y`, `uclick:x,y`, `utab N`, `upadbtn:a,1`, `utype:text` ...)
  are in `port/runtime/platform_sdl.c`, `run_script`. Many modules print traces when a `DL_*`
  environment variable is set (search the sources for `getenv("DL_`).

## How a change usually goes

1. Find the game's function (the generated C in `build/port_gen`, the sources in `src/`, the
   names in `include/darklands/` and `symbols/names.txt`).
2. Hook it in a module of `mods/` or `ui/`, read or change the game's memory (`g_rb`, `g_rw`,
   `rb(G_FAR2E38, ...)`; layouts in `docs/ENGINE.md`, `port/runtime/game.h` and the comments).
3. Give the change a switch (`config_get_int("...", default)`, shown in the F12 menu), and make
   it stay off with "Play the original game" (`config_get_int("original", 0)`).
4. Write a line about it in `docs/release_notes.md` (and `docs/MODDING.md` for modders), test
   it with the tools above, and keep comments to what the code does and why.

## Where to find things

| | |
|---|---|
| players' notes in the release zip | `port/tools/release_readme.txt` |
| what Darklands+ adds, how it works, third-party code | `port/README.md` |
| making mods (data, cards, texts, rules, items, foes, art, sounds) | `docs/MODDING.md` |
| the engine's modules, data layouts, original bugs | `docs/ENGINE.md`, `docs/CARDS.md` |
| file formats | `formats/`, `formats/docs/FORMATS.md` |
| making a release | `docs/RELEASING.md` |
| the decompilation toolchain | `docs/TOOLCHAIN.md`, `docs/REBUILD.md`, `docs/PORTING.md` |
