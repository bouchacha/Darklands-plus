# Native screens: replacing the game's interface a piece at a time

Darklands+ runs the game's own code, recompiled. Its first features were layered over the
picture the game draws: the crisp text, for example, finds the game's letters in the picture
and paints a font over them. That works, but the seams show (letters in the wrong place for
a moment, badges placed by guesswork), and every feature has to work out what the game has
just drawn.

A **native screen** goes further. The game's code still decides everything; only drawing and
input are replaced. Darklands+ reads what the game was about to show from the game's own
data, draws it itself, and hands the player's choice back exactly as the original code
would have. This is the OpenRCT2 route: replace the original one piece at a time, keep the
game playable at every step, and keep a switch that brings back the original for every piece.

The first native screen is the **card screen** (`port/mods/cardview.c`,
`port/ui/cardview.cpp`): F12 > Helpers > "Cards drawn by Darklands+". "Play the original
game" turns it off.

## The card screen, as a worked example

The card engine is overlay s043 150b. Its source, nearly all of it matching C, is
`src/wip/s043/s043_150b.c`. Read its header comment first. The card scripts (s044..s135,
`docs/CARDS.md`) call five functions, and these are the seams:

| engine function | what the original does | the native version |
|---|---|---|
| `card_text` 03b0 (block, how) | shows the picture (palette, frame, card window), draws the text block word-wrapped into the picture, waits for a click or Enter (`wait_continue` 2826) | the same picture steps, then `text_expand` (06fe) into `msg_buf`; the text is read into paragraphs and drawn by `ui/cardview.cpp`; the wait loop is re-implemented |
| `card_options` 049a (block) | draws the option block, waits for a click, fades the picture, recolours it, lays out the option lines into `D_e96e` | the picture, `text_expand`, the text read into pieces; no wait and no fade (the picture is dimmed instead); `D_e96e` is filled from the native layout |
| `card_menu` 1142 (win, win2) | highlights lines under the mouse (`card_mouse` 1744), shows the saints/items pop-up box, returns the option chosen | re-implemented: the same loop (menu bar, portraits, F1..F6, Shift-F1..F5, Alt-Q, Up/Down/Enter), the options found under the pointer in the native layout |
| `sub_13f2` (redraw after a member's sheet) | reloads the picture and draws the text again | the original runs, then the picture is copied over the old text |
| pop-up box (2252 + 203a) | lists each member's saints or items next to an option of kind 1/2 | 2252 still writes the lines and the "choosable" flags (`D_ea92` = the pop-up layout's `en[]`); they are read and drawn under the option |

Rules that keep it faithful:

* **The game's data, not its pixels.** The text is the engine's own `msg_buf` after
  `text_expand`, read with `draw_text_wrapped`'s rules (`src/wip/root/text.c`): option codes
  0x15 / 0x10 (items) / 0x16 (saints) / 0x06 (battle), `opt_on[]` (0 hidden, 1 choosable,
  2 shown greyed), the conditional parts 0x01..0x13 (`D_9c30[]`), and 0x1d after the "..."
  of an option.
* **The same side effects.** Whatever the original sets, the native version sets too:
  `combat_clock`, `D_e959` (block shown), `D_e898`, `D_ee55` and the palette, `menu_member`
  / `menu_slot` / `menu_popup`, `D_8dde`, the pop-up layout `D_ea64` (copied from `D_eb5a` or
  `D_ec50`, because its +0xa6 holds the item and saint handlers the script calls through
  `D_eb0a[menu_slot]`), and the click record (`047c:1320`). The picture-only parts (frame,
  copy into the card window) are the original's own calls.
* **The game's mouse.** The options are tested against the game's pointer
  (`mouse_x` / `mouse_y`, `mouse_buttons`) in game pixels, from rows the drawing code
  publishes each frame (`cardview.row_y0/y1`). So everything that clicks for the player keeps
  working unchanged: the game controller, Go to (`ui/citynav.cpp`), number keys
  (`assist_card_key`) and the test scripts.
* **The rest of Darklands+ sees the same layout.** `D_e96e` (lines, option numbers, kinds,
  y/h) is filled from the native rows, so the chances (`mods/assist.c`), the chronicle and Go to
  read it as before.

Calling game code from a native loop: `gc()` in `cardview.c` (as `gcall` in `encounters.c`)
calls a recompiled function on the game's stack and notices when the game unwinds (a game
loaded from the menu bar, Quit), in which case the hook returns at once and lets it unwind.
Driver thunks in root 047c (for example 059e, the sound driver) may end in interpreted code
that returns to the native caller's sentinel `F000:FFF0`. `runtime/interp.c` (`jump_to`)
treats that as a return to native code.

## Checking it

* `ucmd ucv:` (test command) prints the pieces, their rows and the pop-up entries.
* `DL_CARDVIEW_LOG=1` logs each block, the pop-ups and the option chosen.
* The same choice should give the same result with the screen on and off. The game's
  `rand()` state is at DGROUP 7b20; `ucmd urand:N` sets it. With the seed fixed just before
  a choice, the same seed gave the same outcome on and off. (Without a fixed seed the result
  varies from run to run, on and off alike, because the game draws random numbers while it
  waits.)
* Things tested by hand: text and option cards, light and dark pictures, the drop initial,
  hover / click / number keys / Up-Down-Enter, chances, greyed options, the items list with a
  potion used from it, the saints tooltip, F1 sheets and back, loading a game from a card,
  a battle started from a card, Go to across several cards, the new encounters, and Windows
  (Wine).

## Next screens

The same pattern fits any screen whose logic is separate from its drawing:

* **Shops (barter, s032)** and **loot (s040)**: lists of items with prices; the native
  inventory (F12) already shows the data.
* **The character sheet (member_sheet)** and **equipment (s031)**.
* **Character creation (s036 / s037)**.
* **The travel map's menus** and **the city menus**, which are cards already.

For each: find the functions that draw and the ones that wait for input, keep everything
else, publish the native layout back into the game's own variables, and give it a switch.
