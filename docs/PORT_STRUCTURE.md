# How Darklands+ is put together, and where it is going

The goal is a remake taken apart module by module: each part of the game (cards, shops,
character sheets, loot, the travel map, battles) becomes its own native module, until the
original engine is only a library of rules the modules call. Until then, every step has to
leave the game playable, and every new module keeps a switch back to the original.

## The layers

| layer | where | what it may do |
|---|---|---|
| runtime | `port/runtime/` | the machine the recompiled game runs on: CPU state, DOS, BIOS, sound, the window. Knows nothing of Darklands. |
| game code | `build/port_gen/` (made on the player's machine from DARKLAND.EXE) | the original game, recompiled function by function. Never edited by hand. |
| names | `port/runtime/gamevars.h` | the game's variables (`GV_party_size`) and functions (`GF_barter_screen` = section, segment, offset) by the decompilation's names. **Generated** from `include/darklands/*.h` by `port/tools/gen_gamevars.py`; a new name goes into those headers, so the decompilation and the port share one list. |
| game model | `port/mods/gamemodel.h` | the game's state by meaning: the party, a member's things and what is in use, what they know. Native screens read the game through it. |
| hooks | `port/runtime/hooks.h` | a module replaces or wraps a game function. Hooks on one function **chain** (the last installed runs first; `hook_call_original()` runs the next, and the last runs the game's own). No module needs to know another's hooks. |
| native screens | `port/mods/nscreen.h`, `port/mods/<screen>.c` | a screen of the game run natively: a hook on the original screen function, a model built from the game model, a loop that does what the player asked through the game's own routines (`ns_call`). See `docs/NATIVE_SCREENS.md`. |
| views | `port/ui/<screen>.cpp`, `port/ui/nsdraw.h` | draw a screen from its model alone and turn clicks into actions. They do not call game code. |
| mods and helpers | `port/mods/*.c`, `port/ui/*.cpp` | everything else (fixes, assists, the journal, saves ...), each with its setting. |

## Rules

* **One address, one name.** No raw DGROUP address or `seg:off` in new code: use `GV_` / `GF_`
  (or a game-model accessor). If the name is missing, add it to `include/darklands/game.h`
  and run `gen_gamevars.py`. (The few raw addresses left are far-data offsets not yet named.)
* **The game decides.** A native module asks the game's own routines (prices, buying, equipping,
  a card's outcome); it does not reimplement a rule until the module that owns the rule is taken
  over as a whole, with an on/off comparison (`docs/NATIVE_SCREENS.md`, "Checking it").
* **Views draw, loops act.** Drawing never calls the game; a view pushes an action, the screen's
  loop does it at a safe point (between the game's rounds). Outside a native screen, the interface
  asks with `ns_defer()`: done at the start of the next round of any game screen (menu_bar_run),
  or at the end of the frame while F12's window holds the game (F12 > Party's giving, readying
  and leader change work so). The one exception is a pure question with
  no lasting effect, asked under an evaluation snapshot and kept (the quest log's person names,
  `ui/quests.cpp`); the journal's prices are asked in hooks (a card loaded, a shop drawn).
* **Each module owns its hooks.** If two features need the same game function, each hooks it;
  the chain runs both. (Before the chain, one hook per function forced unrelated features
  into one file.)
* **Every module has a switch,** and "Play the original game" turns all of them off.
* **Check before a release:** `port/tools/release_check.py --game DIR` (and with the ASAN
  build) runs Quickstart, native cards, Go to, the native shop (sell, buy), a sheet opened
  from the shop, a battle with a sheet opened in it, and the loot, and fails on any memory
  error, hook clash or crash.

## The order of the modules

1. Cards (s043) - native since 1.1.
2. Shops (s032) and character sheets (s031) - native in 1.2 (the paper doll stays the game's own).
3. Loot (s040): the same two-list design as the shop - native in 1.2.
4. The character sheet in battle - native in 1.2; then the battle's own screens.
5. Alchemy (s033) - native in 1.2.
6. The travel map and its menus (s030).
7. Battles (s014 and the rest), the largest: model first (combatants, map), then drawing,
   then input, each step comparable with the original.
