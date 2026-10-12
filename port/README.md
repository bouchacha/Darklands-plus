# Native Darklands ("Darklands+")

Darklands running natively on Linux and Windows, without DOSBox, from the player's own copy of the
game (GOG 483.07). Nothing of the game is in this repository: the program's code is translated
from `DARKLAND.EXE` at build time, and the game's data files are read from the game folder at run
time.

## How it works

* **Static recompilation** (`recomp/recomp16.py`): every function the analysis found
  (`build/analysis/analysis.pickle`, see `docs/REBUILD.md`) becomes a C function working on an
  emulated 16-bit machine state (registers, flags, 1 MB of memory plus EMS). Overlays (RTLink)
  are loaded into memory as the game asks for them; the generated code checks which overlay is
  in place. The graphics, sound and misc drivers are recompiled too (`recomp/recomp_drv.py`).
  Code that modifies itself (the PIC decoder) or was not found statically runs in a small
  interpreter (`runtime/interp.c`).
* **DOS and BIOS** are emulated at the interrupt level (`runtime/dos.c`, `bios.c`, `ems.c`):
  files map to the game folder (without regard to case), the timer and keyboard interrupts are
  delivered to the game's own handlers, VGA mode 13h is shown through SDL2
  (`runtime/platform_sdl.c`), the Sound Blaster FM chip is emulated with Nuked OPL3.
* **Hooks** (`runtime/hooks.c`): any game function can be replaced or wrapped
  (`hook_install(section, seg, off, fn)`, `hook_call_original()`); the additions of `mods/` and
  `ui/` are built on them.
* **Interface** (`ui/`): Dear ImGui over the game picture: the F12 menu, the world map, the crisp
  text, the helpers' overlays.

## Building

```
python3 -I tools/build.py /path/to/DARKLAND/DARKLAND.EXE      # the analysis (once)
GAME=/path/to/DARKLAND sh port/tools/gen.sh                   # generated C into build/port_gen
make -C port -j8                                              # Linux: build/port/darklands
make -C port TARGET=win64 -j8                                 # Windows (mingw-w64, SDL2 in build/sdl2-win64)
build/port/darklands /path/to/DARKLAND                        # run
```

## What it adds (all can be switched off; F12 > Helpers > "Play the original game" turns everything off)

* **Text and display:** crisp text everywhere (six typefaces, outlines, a contrast floor),
  words painted into pictures redrawn crisply, character sheet abbreviations spelled out with
  skill icons, smooth scaling options, wide-screen margins with date, place, purse, tasks and the
  party's condition, window size and full screen remembered.
* **Helpers:** chances of success on card options; the saints and items a card accepts; number
  keys for card options; "Go to" in cities (G); shop and inventory tooltips with weapon numbers
  and who could use an item; battle info under the pointer; battles pausing when someone falls;
  battle and dungeon speed; better battle pathfinding; auto-travel and a zoomable world map (M);
  quest log, town journal, saints finder, alchemy formulas, party inventory with drag and drop;
  the party side by side with a one-click leader change; F1 for the keys of the moment (the
  game's own hidden ones too); the mouse wheel in lists; the game waits while in the background.
* **Saves:** quick save / load, autosaves on arrival and before battles, a save browser; after
  the party's end the title comes back instead of the program closing.
* **Fixes** for bugs of 483.07 (weapon potions, vanishing items, quests off the map, garish
  colours, Save/Load greyed out, ...), see the Helpers tab.
* **Built-in mods** (off by default): swift travel, half-price inns, quick learning, heavy blows,
  a strong Quickstart party, a rich start.
* **Modding:** mod folders (data files, single card files, card texts as text, menu texts,
  weapon numbers, items, art, sounds), see `docs/MODDING.md`.

## Testing

`tools/nplay.py` runs a scripted session (`--x11` with the interface drawn, `--wine` for the
Windows build) and takes screenshots; `tools/nctl.py` keeps a session to drive step by step;
`tools/batch.sh` runs the standard play tests. Scripts can use test commands of the platform
layer (`ucmd ucard:`, `ugoto:N`, `uparty:`, `ucbt:` ...; `runtime/platform_sdl.c`, `run_script`).
Environment variables `DL_*` switch on traces (see the sources).
