Darklands+ runs Darklands (MicroProse, 1992) natively on Windows, without DOSBox, from your own copy of the game. It adds the things players have asked for over the years, and every addition can be switched off. **F12 > Helpers > "Play the original game"** turns all of them off at once.

**You need** your own Darklands 483.07 (GOG or Steam) and 64-bit Windows 10 or 11. This download contains nothing from the game.

**Install:** unpack the zip anywhere and start `darklands.exe`. It finds a GOG or Steam installation by itself or asks for the folder once. The first start takes about two minutes while Darklands+ translates the game program from your `DARKLAND.EXE`; after that it starts at once. Your saved games stay where they always were. `Darklands+ README.txt` in the zip has the details.

### What it adds

- **Text and display:** crisp text at any window size up to 4K, in a choice of six typefaces. The pointer follows the mouse smoothly. Wide-screen margins show the date, place, purse and tasks on one side, and on the other the town you are in or near (reputation, buildings, shops and their quality), the blessings and potions at work and who carries too much. One setting sizes the margins, tooltips and map names. Full screen with Alt+Enter. PNG screenshots with Ctrl+F12.
- **Tooltips everywhere:** point at an item, a saint or an alchemy formula on any of the game's screens (shops, loot, character sheet, equipment, saints lists, the formula book, a card's list of saints and items) to see what it does. Saint descriptions show the effect in its own colour.
- **Cards:** chances of success next to the options (only where the party's skills decide). Number keys pick options. The saints and items a card accepts are shown. "Go to" in cities (G) walks the party to a shop, church or gate.
- **Journal:** a quest log, a town journal, a saints finder, alchemy formulas, and where to find goods and ingredients.
- **Travel:** a zoomable world map (M) and auto-travel: Ctrl+click a place and the party goes there.
- **Battles:** a battle log, info under the pointer, a pause when someone falls, adjustable battle speed, better pathfinding, and a map of the level (Tab).
- **Party and inventory:** the whole party side by side with drag and drop, item tooltips with weapon numbers and who can use them, and a one-click leader change. On the loot screen, A takes the whole pile and S shares it out among the party, instead of one G per arrow.
- **Saves:** quick save (Ctrl+S) and quick load (F9), autosaves on arrival and before battles, and a save browser.
- **Fixes for bugs of 483.07:**
  - blessings that cost attribute points for good
  - items lost to a full pack
  - "(null)" items
  - the "neon hair" colours
  - Save and Load greyed out after the inn
  - weapon potions that never wear off
  - quests off the edge of the map
  - robbers corrupting the item count (a crash later)
  - and more, each with its own switch (F12 > Helpers)
- **Restored:** the Honor Roll of Glory, drawn by MicroProse but never reachable.
- **Built-in mods** (off by default): swift travel, half-price inns, quick learning, heavy blows, a strong Quickstart party, a rich start.
- **Modding:** mod folders for game data, single card texts, menu texts, weapon numbers, items, pictures and sounds. F12 > Mods makes a new mod folder with instructions.

### If something goes wrong

Darklands+ writes `darklands_port.log` in the game folder; please attach it to a report. Some antivirus programs distrust files a compiler has just made. If yours blocks `darklands_game.dll`, allow it; it is made from your own `DARKLAND.EXE` by the TinyCC in `darklands_plus\tcc`. On Linux and Steam Deck the Windows zip runs under Wine or Proton.
