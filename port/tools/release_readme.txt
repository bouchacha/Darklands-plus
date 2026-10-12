Darklands+ @VERSION@ for Windows (64-bit)
=========================================

Darklands (MicroProse, 1992) running natively on Windows, without DOSBox, with optional
improvements: crisp text up to 4K, a zoomable world map with auto-travel, chances of success
on card options, a quest log and town journal, a battle log, pausing when someone falls,
better battle pathfinding, quick saves and autosaves, fixes for bugs of the original game, and
mod folders for items, card texts, pictures, sounds and rules.

Everything new can be switched off. F12 > Helpers > "Play the original game" turns all of it
off at once.

WHAT YOU NEED
Your own copy of Darklands, version 483.07 (the GOG release). This download does not include
anything from the game.

INSTALLING
1. Unpack this zip into your Darklands folder, the one that contains DARKLAND.EXE (for GOG
   usually C:\GOG Games\Darklands, or its DARKLAND subfolder). This adds darklands.exe and
   the darklands_plus folder. The original game files are not changed.
2. Back up your SAVES folder. Saves go into the game folder, as in the original.
3. Double-click darklands.exe.

THE FIRST START takes a few minutes. Darklands+ translates the game program from your
DARKLAND.EXE and keeps the result as darklands_game.dll in the game folder. After that it
starts at once. It does this again only after an update that changes the translation.

KEYS
  F1            keys and clicks       M           world map (travel map or a card)
  F12           Darklands+ menu
  Ctrl+S        quick save            F9, Ctrl+L  quick load
  Alt+Enter     full screen           Ctrl+click  (travel map) travel there
  Ctrl+F12      screenshot            G           (city card) go to a place
  1-9           (card) pick an option
  Mouse wheel   (sheet, shops) move through a list
  Tab           (battles, dungeons) the level map
  Right click   the menu bar

MAKING MODS
  mods\My mod\mod.txt               first line: the name shown in F12 > Mods
  mods\My mod\DARKLAND.LST          any game file, replaced whole (items, saints, cities ...)
  mods\My mod\MSGFILES\$MAINS01.TXT one card's text (the other cards stay)
  mods\My mod\text.txt              menu lines said differently (translations)
  mods\My mod\rules.ini             weapon numbers and other rules
  mods\My mod\art\, sounds\         replacement pictures and sounds
  F12 > Mods > "Make a new mod folder" makes one with a mod.txt that explains this.

IF SOMETHING GOES WRONG
Darklands+ writes darklands_port.log into the game folder. Include that file when you report
a problem. If the first start fails, the log says why. To redo the translation, delete
darklands_game.dll.

To remove Darklands+, delete darklands.exe, darklands_game.dll, darklands_port.ini,
darklands_port.log and the darklands_plus folder.

WHAT IS IN darklands_plus
  src\      the translator (Python), the part of Darklands+ that reads DARKLAND.EXE
  python\   Python for Windows (embeddable package, PSF licence)
  tcc\      TinyCC, the C compiler that builds darklands_game.dll (LGPL 2.1)
  licenses\ the licences of everything in this download
