# Darklands+

**A free, unofficial update for *Darklands* (MicroProse, 1992), the role-playing game of 15th-century Germany.** Darklands+ runs your own copy of the game natively on modern Windows, without DOSBox. It keeps the game as it was and adds what it always lacked: screens made for the mouse, the information the game kept hidden, less busywork, and fixes for its bugs. Every change can be switched off.

> Darklands+ is not a remake and does not contain the game. You need your own copy of Darklands from GOG or Steam.

<p>
<img width="49%" alt="A card drawn by Darklands+: crisp text, chances of success, the party and the town beside it" src="https://github.com/user-attachments/assets/29a43f4c-0ce2-49cf-9259-314aeca98485" />
<img width="49%" alt="A battle with its log and the enemies' state" src="https://github.com/user-attachments/assets/d66df731-a125-4f05-93ec-c9dedd4256a7" />
<img width="49%" alt="The character sheet: skills, with the whole party compared" src="https://github.com/user-attachments/assets/3d781e1a-8ae0-446a-889c-401f86e1caf2" />
<img width="49%" alt="The world map: find a place, see how far it is, travel there" src="https://github.com/user-attachments/assets/1e8f79bf-ed3a-4dc9-9d79-0433c587f512" />
<img width="49%" alt="Go to: every place in a city, with what it teaches, sells and offers" src="https://github.com/user-attachments/assets/a4c11405-f2b7-4cc5-8c06-11e76b55a809" />
<img width="49%" alt="A shop rebuilt for the mouse" src="https://github.com/user-attachments/assets/dcd05e9f-c89f-4b01-9053-af56b7177452" />
<img width="49%" alt="Alchemy: each formula's ingredients, how many the party has, and the chance of success" src="https://github.com/user-attachments/assets/45b148ee-e593-41ec-be9e-23ce6151c342" />
<img width="49%" alt="Calling on a saint: what the saint really does, the favor to spend and the chance" src="https://github.com/user-attachments/assets/5bbf5bd5-6469-473c-85c3-1e34ada84bf6" />
</p>

## What is Darklands?

An open-world role-playing game from 1992. You lead a party of up to five through the Holy Roman Empire around 1400: real cities, real saints, alchemy that follows medieval beliefs, and battles in real time with pause. There is no single plot; you make your name with quests, robber knights, witches' Sabbats and the occasional dragon. It is loved for its depth and notorious for its clumsy interface, hidden rules and bugs. Darklands+ is about the second part.

## What Darklands+ changes

- **It runs on today's PCs.** Natively on Windows 10 and 11, in a window of any size or full screen, with crisp text up to 4K. The Roland MT-32 music is built in (with your own ROM files), and game controllers and the Steam Deck work.
- **Screens made for the mouse.** The cards, shops, character sheet, loot, inn, alchemy, saints, saving and loading are drawn anew: click an option, drag an item, point at anything for an explanation.
- **What the game kept hidden.** The chance of success beside each option, why an option is greyed, what every saint and potion really does (with numbers), and what a foe's armour stops.
- **Less busywork.** The party's best member takes each test the game puts to the leader, so you no longer swap leaders. One pack serves the whole party. Go to walks you through a town, and the world map travels to any place you pick.
- **Quests you can follow.** Each task has a title, a deadline and a marker on the map. A journal keeps every text the game has shown you, a record of each town, and your own notes.
- **Better battles.** A log of every blow, fast-forward, smarter pathfinding, a click on a foe to attack it, and a map of the level.
- **Bugs fixed.** 57 fixes for bugs of the original, and 43 saints that now do what their descriptions say.
- **What MicroProse left unfinished.** The Honor Roll of Glory the game never showed, an ambush by soldiers of a private war ("Fehde!"), and two experimental options, off at first: a whole political campaign written and coded but never switched on (take the rebels' side or the rulers' in a city in turmoil, sway the guilds, and march on the town hall or arrest the ringleaders), and cities preparing for war or under siege, with guarded gates, patrols on the walls and soaring prices. [What else the game holds unused](docs/cut_content.md).
- **Mods.** Two come with it: real battle sounds and sixteen new wilderness encounters. You can make your own (items, card texts, pictures, sounds, rules).

**You decide how much changes.** F12 > Settings > Helpers offers presets, from *Buggy Purist* (the original game exactly, bugs and all) and *Purist* (its bugs fixed) to *Easy*, and every switch one by one.

## Install

1. You need your own **Darklands 483.07** (GOG or Steam) and **64-bit Windows 10 or 11**. On Linux and Steam Deck it runs through Proton or Wine.
2. Download the zip from **[Releases](https://github.com/bouchacha/Darklands-plus/releases/latest)**, unpack it anywhere and run `darklands.exe`.
3. It finds your GOG or Steam installation by itself, or asks for the folder once. The first start takes a minute or two while Darklands+ prepares the game; after that it starts at once.

**From your Steam library:** in Steam, *Games > Add a Non-Steam Game to My Library*, *Browse* to `darklands.exe`, and add it.

**On a Steam Deck:** in Desktop Mode, unpack the zip into the Darklands folder (`steamapps/common/Darklands`), add `darklands.exe` as a non-Steam game as above, then in its *Properties > Compatibility* tick *Force the use of a specific Steam Play compatibility tool* and choose Proton. Back in Gaming Mode it is in your library, and the controller works.

## Questions

- **Do my saved games work?** Yes. They stay where they are, in the game's own format, and still load in the original game (unless a character uses one of the extra figures Darklands+ offers). The first start of each version also copies them into a "Darklands+ save backups" folder.
- **Does it go online?** No. Darklands+ never connects to the internet and collects nothing.
- **My antivirus complains.** On the first start Darklands+ builds the game module, `darklands_game.dll`, from your own `DARKLAND.EXE`; some antivirus programs distrust files that were just compiled. Allow it.
- **Mac?** Not natively. Wine-based tools may work but are untested.
- **Something went wrong.** `darklands_port.log` in the game folder says why a start failed. See also `Darklands+ README.txt` in the download.
- **Where do I report bugs?** On GitHub, under [Issues](https://github.com/bouchacha/Darklands-plus/issues). Attach `darklands_port.log` from the game folder and, if you can, a save from just before the problem. The full list of features is in the [release notes](docs/release_notes.md), and [CHANGELOG.md](CHANGELOG.md) says what changed.

## Status

Actively developed. Bug reports, suggestions and contributions are welcome (see [CONTRIBUTING.md](CONTRIBUTING.md)). The author directs and play-tests; the code is written by an AI (Anthropic's Claude). See [LICENSE](LICENSE).

## For developers

Darklands+ rests on a matching decompilation of Darklands 483.07: 382 of the game's source files are C that Microsoft C 6.00A compiles back to the original bytes, and `port/` recompiles the whole game to run natively. This repository holds no game files and no code generated from them. Start with [docs/DEVELOPERS.md](docs/DEVELOPERS.md), [CONTRIBUTING.md](CONTRIBUTING.md), [docs/MODDING.md](docs/MODDING.md) and [docs/MERGING.md](docs/MERGING.md).
