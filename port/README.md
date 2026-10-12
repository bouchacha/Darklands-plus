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
python3 -I tools/analyze.py /path/to/DARKLAND/DARKLAND.EXE build/analysis   # the analysis (once; needs capstone)
GAME=/path/to/DARKLAND sh port/tools/gen.sh                   # generated C into build/port_gen
make -C port -j8                                              # Linux: build/port/darklands
make -C port TARGET=win64 -j8                                 # Windows (mingw-w64, SDL2 in build/sdl2-win64)
build/port/darklands /path/to/DARKLAND                        # run
```

## Releases: the game's code made on the player's machine

The program above contains code translated from `DARKLAND.EXE`, so it cannot be passed on. A
release instead ships the program without that code and makes it on first start, from the
player's own copy (`runtime/split.h`):

```
make -C port SPLIT=1 TARGET=win64 -j8      # the program alone (no game files needed): build/port_win64_split
python3 -I port/tools/prepare.py --game /path/to/DARKLAND --out /path/to/DARKLAND/darklands_game.dll
```

`prepare.py` is what the program runs when the module is missing or was made by another
translator (a progress window; a minute or two): the analysis, the translation and TinyCC,
which compiles the result into `darklands_game.dll` (`libdarklands_game.so` on Linux, where
`make -C port SPLIT=1` and `make -C port module` work the same way). The translator's id is the
hash of the files in `port/tools/translator_files.txt`; both the program and the module carry it.

`port/tools/package.py` makes the Windows zip (program, translator, TinyCC from
`port/tools/build_tcc_win.sh`, the embeddable Python with capstone, licences; it refuses to pack
anything made from the game), and `.github/workflows/release.yml` does all of it on GitHub for a
tag `v...`. The zip's contents go into the Darklands folder; `port/tools/release_readme.txt` is
the players' README.

## What it adds (all can be switched off; F12 > Helpers > "Play the original game" turns everything off)

On the first start a short welcome names the keys that matter and offers the original game untouched.

* **Text and display:** crisp text everywhere, sharp up to 4K (six typefaces, outlines, a contrast floor),
  words painted into pictures redrawn crisply, character sheet abbreviations spelled out with
  skill icons, smooth scaling options, wide-screen margins with date, place, purse, tasks and the
  party's condition (in battles the battle log and everyone's condition), window size and full screen remembered; on Windows DPI-aware, the menus
  sized by the system's scale; the game rests while it only waits for input (an idle card
  takes about a tenth of a core).
* **Helpers:** chances of success on card options (only where the party's skills decide); the saints and items a card accepts; number
  keys for card options; "Go to" in cities (G); shop and inventory tooltips with weapon numbers
  and who could use an item; battle info under the pointer; battles pausing when someone falls;
  a battle log (every blow, wound and fall); battle and dungeon speed; better battle pathfinding; auto-travel and a zoomable world map (M);
  quest log, town journal, saints finder, alchemy formulas, party inventory with drag and drop;
  the party side by side with a one-click leader change; F1 for the keys of the moment (the
  game's own hidden ones too); the mouse wheel in lists; the menu bar with a right click (the
  game shows it only while the button is held); the game waits while in the background;
  a map of the level in battles and dungeons (Tab).
* **Saves:** quick save / load, autosaves on arrival and before battles, a save browser; after
  the party's end the title comes back instead of the program closing.
* **Screenshots:** Ctrl+F12 saves the window as it is seen (crisp text, margins) as a PNG in the
  screenshots folder.
* **Fixes** for bugs of 483.07 (weapon potions, vanishing items, quests off the map, garish
  colours, Save/Load greyed out, robbers corrupting the item count - a crash when that
  character later dies in battle, ...), see the Helpers tab.
* **Restored:** the Honor Roll of Glory (hall of fame), drawn and half-written by MicroProse but
  never reachable; parties enter it at their journey's end (`ui/halloffame.cpp`).
* **Built-in mods** (off by default): swift travel, half-price inns, quick learning, heavy blows,
  a strong Quickstart party, a rich start.
* **Modding:** mod folders (data files, single card files, card texts as text, menu texts,
  weapon numbers, items, art, sounds), see `docs/MODDING.md`.

## Testing

`tools/nplay.py` runs a scripted session (`--x11` with the interface drawn, `--wine` for the
Windows build) and takes screenshots; `tools/nctl.py` keeps a session to drive step by step;
`tools/batch.sh` runs the standard play tests; `tools/smoke.py` checks a build quickly (start,
battle, save and load); `tools/monkey.py` plays at random for as long as asked (a soak test: it
reports a crash or a session that stops changing). The game folder comes from `--game` or
`DARKLANDS_DIR`; it is copied, never changed. Scripts can use test commands of the platform
layer (`ucmd ucard:`, `ugoto:N`, `uparty:`, `ucbt:` ...; `runtime/platform_sdl.c`, `run_script`).
Environment variables `DL_*` switch on traces (see the sources).

## Third-party code

| component | where | licence |
|---|---|---|
| Dear ImGui 1.91.5 | `third_party/imgui` | MIT (`LICENSE.txt`) |
| Nuked OPL3 (FM sound) | `third_party/nuked-opl3` | LGPL 2.1 or later (`LICENSE`) |
| stb_image, stb_image_write, stb_vorbis | `third_party/stb` | public domain or MIT (in each file) |
| fonts: EB Garamond, Alegreya, Atkinson Hyperlegible, IM Fell English, MedievalSharp, UnifrakturMaguntia ("DL Fraktur") | `third_party/fonts` | SIL Open Font License 1.1 (`fonts/README.txt`) |
| SDL2 | linked (Linux: the system's; Windows: `build/sdl2-win64`, static) | zlib |

Whoever distributes builds should ship these notices with them; Nuked OPL3's LGPL also asks
that users can relink the program with their own version of it, which the published source
allows.
