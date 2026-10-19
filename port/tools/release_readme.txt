Darklands+ @VERSION@ for Windows (64-bit)
=========================================

Darklands (MicroProse, 1992) running natively on Windows, without DOSBox, with optional
improvements: most of the game's screens rebuilt for the mouse (cards, shops, the character
sheet, loot, the inn, alchemy, saints, save and load ...), the best member making the card
tests put to the leader, one pack for the whole party, quests with titles, deadlines and
pictures on the maps, crisp text up to 4K, a zoomable world map with auto-travel, chances of
success on card options, a town journal, a battle log, better battle pathfinding, quick saves
and autosaves, fixes for bugs of the original game, and mod folders for items, card texts,
pictures, sounds and rules. What is new since 1.0: CHANGELOG.md on the GitHub page.

Everything new can be switched off. The first start asks how you would like to play, from
"Buggy Purist" (the game exactly as it was) and "Purist" (its bugs fixed) to "Darklands+" and
"Easy"; F12 > Settings > Helpers changes it, or any switch one by one.

WHAT YOU NEED
Your own copy of Darklands, version 483.07 (as GOG and Steam sell it). This download does not
include anything from the game.

INSTALLING
1. Unpack this zip into a folder of its own, for example C:\Games\Darklands+. (You can also
   unpack it into your Darklands folder, the one that contains DARKLAND.EXE.)
2. Back up your SAVES folder. Saves stay in the Darklands folder, as in the original. (If
   Windows does not let programs change that folder - GOG Galaxy installs under Program
   Files - the game's saves and other files go to the VirtualStore folder Windows keeps for
   it, %LOCALAPPDATA%\VirtualStore\...; saves that GOG's DOSBox was redirected there show
   up too. F12 > Settings > About shows the place.)
3. Double-click darklands.exe. Darklands+ finds a GOG or Steam installation by itself;
   otherwise it asks you for the Darklands folder once. The original game files are not
   changed. Windows may warn about a program from an unknown publisher (Darklands+ is not
   signed): choose "More info", then "Run anyway".

The opening plays at every start: a key or a click goes on to its last scene, the lady's
welcome, and another to the start screen (F12 > Settings > Display: the whole opening, only its
last scene, or nothing). The start screen shows the version; Continue there loads your newest
save.

THE FIRST START takes a minute or two (longer on an older computer). Darklands+ translates
the game program from your DARKLAND.EXE and keeps the result as darklands_game.dll in the
game folder. After that it starts at once. It does this again only after an update that
changes the translation.

KEYS
  F1            keys and clicks       M           world map (travel map or a card)
  F12           Darklands+ menu
  Ctrl+S        quick save            F9, Ctrl+L  quick load
  Alt+Enter     full screen           Ctrl+click  (travel map) travel there
  Ctrl+F12      screenshot (PNG in the screenshots folder of the Darklands folder; it also
                writes the window's sizes into darklands_port.log, useful in a report)
  Click a foe   (battles) the selected member attacks it, even before an attack is chosen
                (one better with a bow than a blade shoots, when the line is clear)
  1-5           (battles) choose a member; twice: the view goes to them
  Right drag    (battles) move the view (a right click alone opens the menus)
  Alt           (battles) a bar over every fighter, not only the hurt
  Click a column heading  (Darklands+ tables) sort by it
  G             (city card) go to a place
  1-9           (card) pick an option
  Mouse wheel   (sheet, shops) move through a list
  Tab           (battles, dungeons) the level map
  Z             (battles) fast forward, six times the pace; Z again: back to the chosen speed
  Right click   the menu bar (click an entry; a click elsewhere or any key closes it)
  A, S          (loot) take the whole pile, share it out among the party
  A, T          (shop) sell the whole stack, buy ten
  A controller: left stick the pointer, A click, B right click, X Enter, Y Esc, d-pad arrows,
                Start F12, Back M, LB Tab, RB Space, right trigger held Ctrl (F1 shows them)

ROLAND MT-32 MUSIC (strongly recommended)
Darklands was composed for the Roland MT-32 / LAPC-I: its music and battle sounds are made with
instruments the game loads into that synthesizer (flutes, lutes, the clash of blades). The Sound
Blaster version is a stand-in. Darklands+ has Munt's emulator of the MT-32 built in, but it
needs the synthesizer's two ROM files (control and PCM, from an MT-32, CM-32L or LAPC-I), which
belong to Roland and are not included. If you use Munt or DOSBox with an MT-32 you have them
already: Darklands+ finds them by their contents (any file names) in Munt's ROM folder, in
DOSBox Staging's mt32-roms folder, or in a "roms" folder next to darklands.exe or in the
Darklands folder. When none are found, Darklands+ says so when it starts and lets you choose
their folder (or "Don't remind me again"); F12 > Settings > Display > Sound can name the folder too. With
both a CM-32L and an MT-32 pair the MT-32 plays (on the CM-32L the battle sounds turn into
chimes); you can switch between them there while playing.

BATTLE SOUNDS
The mods folder of this download holds "Battle sounds (0 A.D. recordings)": real sounds of
swords, shields, blows, cries, the bow and the handgun in place of the synthesizer notes. Tick
it in F12 > Settings > Mods (F12 > Settings > Display sets the volume). The recordings are from 0 A.D. by Wildfire
Games, CC BY-SA 3.0 (mods\Battle sounds\CREDITS.txt).

NEW ENCOUNTERS
The mods folder also holds "New encounters": sixteen new wilderness encounters (the gallows
oak, a herb-wife, flagellants, a robber knight's toll bridge, kobolds at a mine, deserters,
a knight's feud and more),
balanced against the game's own and coming up among them. Tick it in F12 > Settings > Mods; a small red
seal marks a new encounter's card. F12 > Settings > Mods sets how often they come up.

MAKING MODS (in the Darklands folder)
  mods\My mod\mod.txt               first line: the name shown in F12 > Settings > Mods
  mods\My mod\DARKLAND.LST          any game file, replaced whole (items, saints, cities ...)
  mods\My mod\MSGFILES\$MAINS01.TXT one card's text (the other cards stay)
  mods\My mod\text.txt              menu lines said differently (translations)
  mods\My mod\rules.ini             weapon numbers and other rules
  mods\My mod\art\, sounds\         replacement pictures and sounds
  mods\My mod\music\                recordings in place of the game's music, named by the
                                    piece's number (10.ogg, "11 market.ogg"; F12 > Settings > Display)
  mods\My mod\ENCOUNTERS\           new encounters as scripts (see mods\New encounters)
  F12 > Settings > Mods > "Make a new mod folder" makes one with a mod.txt that explains this.

IF SOMETHING GOES WRONG
Darklands+ writes darklands_port.log into the game folder. If the first start fails, the log says
why. (There is no official support: the project has been handed to the community, so look for
the active fork, or fix it yourself: the source is on GitHub.) To redo the translation, delete
darklands_game.dll. Some antivirus programs distrust files that a compiler has just made: if
yours removes or blocks darklands_game.dll, allow that file (it is made from your own
DARKLAND.EXE by the TinyCC in darklands_plus\tcc).

To remove Darklands+, delete the folder you unpacked it into, and darklands_game.dll,
darklands_port.ini, darklands_port.log, darklands_journal_*.txt, darklands_chronicle_*.txt,
darklands_notes_*.txt and darklands_sabbats_*.txt from the Darklands folder (or the folder
%LOCALAPPDATA%\Darklands+, where they go when the Darklands folder cannot be written).

WHAT IS IN darklands_plus
  src\      the translator (Python), the part of Darklands+ that reads DARKLAND.EXE
  python\   Python for Windows (embeddable package, PSF licence)
  tcc\      TinyCC, the C compiler that builds darklands_game.dll (LGPL 2.1)
  (darklands.exe contains Munt's libmt32emu, LGPL 2.1; its source is in Darklands+'s source,
  port/third_party/mt32emu)
  licenses\ the licences of everything in this download
