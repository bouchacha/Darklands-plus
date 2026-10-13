Darklands+ runs Darklands (MicroProse, 1992) natively on Windows, without DOSBox, from your own copy of the game. It adds the things players have asked for over the years, and every addition can be switched off. **F12 > Helpers > "Play the original game"** turns all of them off at once.

**You need** your own Darklands 483.07 (GOG or Steam) and 64-bit Windows 10 or 11. This download contains nothing from the game.

**Install:** unpack the zip anywhere and start `darklands.exe`. It finds a GOG or Steam installation by itself or asks for the folder once. The first start takes about two minutes while Darklands+ translates the game program from your `DARKLAND.EXE`; after that it starts at once. Your saved games stay where they always were. `Darklands+ README.txt` in the zip has the details.

### What it adds

- **Text and display:** crisp text at any window size up to 4K, in a choice of seven typefaces (a readable gothic one, Grenze Gotisch, by default). The pointer follows the mouse smoothly. Wide-screen margins show the date, place, purse and tasks on one side, and on the other the town you are in or near (reputation, buildings, shops and their quality), the blessings and potions at work and who carries too much. One setting sizes the margins, tooltips and map names. The window opens maximized (it remembers if you make it smaller). Full screen with Alt+Enter. PNG screenshots with Ctrl+F12.
- **Tooltips everywhere:** point at an item, a saint or an alchemy formula on any of the game's screens (shops, loot, character sheet, equipment, saints lists, the formula book, a card's list of saints and items) to see what it does. Saint descriptions show the effect in its own colour.
- **Cards:** chances of success next to the options (only where the party's skills decide). Number keys pick options. The saints and items a card accepts are shown. "Go to" in cities (G) walks the party to a shop, church or gate.
- **Journal:** a chronicle of every text the cards have shown (word for word, with the date, place and your choice, searchable), your own notes for each game, a quest log, a town journal, a saints finder, alchemy formulas, where to find goods and ingredients, and what the party has left in store rooms anywhere.
- **Travel:** a zoomable world map (M) and auto-travel: Ctrl+click a place and the party goes there. Towns and places are named on the travel map. Random encounters on the road can be made rarer (or more frequent).
- **Battles:** a battle log, info under the pointer (a foe's weapons, its armour and whether each of your weapons pierces it, is halved or deflected), a pause when someone falls, adjustable battle speed, better pathfinding, and a map of the level (Tab).
- **Party and inventory:** the whole party side by side with drag and drop, item tooltips with weapon numbers and who can use them, a one-click leader change, and "Tidy the pack" (sorts a member's or everyone's items by kind and name). On the loot screen, A takes the whole pile and S shares it out among the party, instead of one G per arrow; in shops, A sells a whole stack and T buys ten.
- **More figures for the party:** Select Character Image offers the enemies' human figures too: the grey-haired, brown-haired and blond ex-adventurers, a sergeant, town guard, mercenary, bandit, knight, knight in plate, Templar, monk, alchemist, witch and cultist (18 figures instead of 4). Each walks, fights and dies with its own animations and has colours of its own for the 1st/2nd/3rd color keys (red, blue, green, gold ...). A figure without an animation for the weapon in hand swings the nearest one it has.
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
  - items lost in overfull store rooms
  - and more, each with its own switch (F12 > Helpers)
- **Restored:** the Honor Roll of Glory, drawn by MicroProse but never reachable.
- **Built-in mods** (off by default): swift travel, half-price inns, quick learning, heavy blows, a strong Quickstart party, a rich start.
- **Roland MT-32 music:** Darklands was composed for the MT-32 / LAPC-I, and Munt's emulator is now built in. Give it the synthesizer's two ROM files (if you use Munt or DOSBox with an MT-32 you have them; Darklands+ finds them in Munt's and DOSBox Staging's ROM folders by itself) and the music and battle sounds play as composed, with no MIDI setup. Without the ROMs the emulated Sound Blaster plays, and Darklands+ explains at start why the MT-32 is worth it and lets you pick the ROMs' folder (with "Don't remind me again"). With both a CM-32L and an MT-32 pair you can switch between them while playing. What the game writes on the synthesizer's display ("[ MPS Darklands ]") is shown for a moment, as on the real one.
- **Enemies (F12 > Records):** every foe with its attributes, weapon skills, armour and how hard it is (which weapons pierce it), shield, missile and melee weapons; editable and saved as a mod.
- **Game controllers:** an Xbox, PlayStation or Switch pad, or a Steam Deck, plays the game: the left stick moves the pointer, A clicks, B opens the menu bar, X / Y are Enter / Esc, the d-pad the arrow keys, Start the Darklands+ menu, Back the world map (F1 lists them all). Off in F12 > Helpers.
- **Real MT-32s and other MIDI synthesizers:** the "Roland MIDI" setting can send to any MIDI output Windows has (a real MT-32 on a MIDI interface, Munt's driver, a Sound Canvas), chosen in F12 > Display.
- **Music volume:** its own slider (F12 > Display), for the FM and the MT-32 alike.
- **Music packs:** a mod (or a "music" folder) can replace the game's pieces of music with recordings (Ogg or WAV), piece by piece; F12 > Display > Sound names each piece as it starts. None comes with Darklands+.
- **Battle sounds:** a mod with real recordings (swords clashing, shields, blows, cries, the bow, the handgun, a war horn for an ambush) in place of the synthesizer notes comes with the download; tick it in F12 > Mods. The recordings are from 0 A.D. (CC BY-SA 3.0).
- **Modding:** mod folders for game data, single card texts, menu texts, weapon numbers, items, pictures and sounds; mods switch on and off at once, sounds included. F12 > Mods makes a new mod folder with instructions.

### If something goes wrong

Darklands+ writes `darklands_port.log` in the game folder; please attach it to a report. Some antivirus programs distrust files a compiler has just made. If yours blocks `darklands_game.dll`, allow it; it is made from your own `DARKLAND.EXE` by the TinyCC in `darklands_plus\tcc`. On Linux and Steam Deck the Windows zip runs under Wine or Proton.
