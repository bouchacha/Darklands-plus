# Darklands+

**A free, unofficial update for *Darklands* (MicroProse, 1992), the role-playing game of 15th-century Germany.** Darklands+ runs your own copy of the game natively on modern Windows, without DOSBox. It keeps the game as it was and adds what it always lacked: screens made for the mouse, the information the game kept hidden, less busywork, and fixes for its bugs. Most changes can be switched off.

> Darklands+ is not a remake and does not contain the game. You need your own copy of Darklands from GOG or Steam.

<p>
<img width="49%" alt="A card: the chance beside each option, who in the party tries it and what it rests on" src="https://github.com/user-attachments/assets/4e1414e0-0255-41a8-8e98-5df6d034efbc" />
<img width="49%" alt="A shop: what a piece of armor would do to the member's load" src="https://github.com/user-attachments/assets/fe17b8b3-7327-4f29-886b-83e165445d69" />
<img width="49%" alt="Alchemy: each formula's ingredients, how many the party has, and the chance of success" src="https://github.com/user-attachments/assets/449549f7-8d03-427b-876c-3ec72f355609" />
<img width="49%" alt="Go to: every place in a city, with what it teaches, sells and offers" src="https://github.com/user-attachments/assets/fdbcae88-95ba-4e86-9604-0947fecc79b8" />
<img width="49%" alt="The world map: every quest listed and marked, several at one place on an arc over it" src="https://github.com/user-attachments/assets/d66192bd-a052-45d1-9521-ef1aee1f3ba2" />
<img width="49%" alt="The character sheet: skills and attributes as heraldic shields, the whole party compared" src="https://github.com/user-attachments/assets/a8c2b7db-4f9c-407d-a6bb-e6d78f6aa94d" />
<img width="49%" alt="A battle: the party fights on between orders, the orders as buttons at the foot of the field, a log of every blow" src="https://github.com/user-attachments/assets/543a7d52-c5d9-4eca-9108-63e2fd9d01b7" />
<img width="49%" alt="Calling on a saint: what the saint really does, the favor to spend and the chance" src="https://github.com/user-attachments/assets/1f556415-549b-48be-8969-5944dd12794f" />
</p>

## What is Darklands?

An open-world role-playing game from 1992. You lead a party of up to five through the Holy Roman Empire around 1400: real cities, real saints, alchemy that follows medieval beliefs, and battles in real time with pause. There is no single plot; you make your name with quests, robber knights, witches' Sabbats and the occasional dragon. It is loved for its depth and notorious for its clumsy interface, hidden rules and bugs. Darklands+ takes on the interface, the hidden rules and the bugs.

## What Darklands+ changes

- **It runs on today's PCs.** Natively on Windows 10 and 11, in a window of any size or full screen, with crisp text up to 4K. The Roland MT-32 music is built in (with your own ROM files), the whole soundtrack can be played from the settings, and game controllers and the Steam Deck work.
- **Screens made for the mouse.** The cards, shops, character sheet, loot, inn and camp, alchemy, saints, saving and loading are redrawn: click an option, drag an item, point at anything for an explanation. Skills and attributes are heraldic shields wherever they appear.
- **What the game kept hidden.** The chance of success beside each option, who in the party will try it and what it rests on, why an option is grayed, what every saint and potion really does (with numbers), what an enemy's armor stops, and what a piece of armor would do to a member's load before you buy it.
- **Less busywork.** The party's best member takes each test the game puts to the leader, so you no longer swap leaders. One pack serves the whole party, and a shop's Sell spare gear also sells whatever you have marked Sell. Go to walks you through a town; from the world map the party travels to any place you pick, and you can look up where to buy alchemical ingredients or any ware.
- **Quests you can follow.** Each quest has a title, a deadline and a marker on the map, the options on the cards that lead to it carry its picture, and the world map (M) lists them all, with a record of each town. A journal keeps every text the game has shown you, and your own notes.
- **Better battles.** The party fights on sensibly between orders: shooters open fire by themselves, step aside for a clear line and join a friend's melee instead of shooting into it. Orders take one click, from buttons at the foot of the field. A log of every blow, fast-forward, smarter pathfinding and a map of the level.
- **Bugs fixed.** 60 fixes for bugs of the original, among them 43 saints that now do what their descriptions say.
- **What MicroProse left unfinished**, on a page of its own (F12 > Reconstructed Content), experimental and off unless you tick it: a whole political campaign written and coded but never switched on (take the rebels' side or the rulers' in a city in turmoil, sway the guilds, and march on the town hall or arrest the ringleaders), cities preparing for war or under siege, with guarded gates, patrols on the walls and soaring prices, an ambush by soldiers of a private war ("Fehde!"), a town's hospice, Black Cloud in battle and the Honor Roll of Glory the game never showed. [What else the game holds unused](docs/cut_content.md).
- **Ironman**, if you want it: one save that the game keeps itself, with no loading back.
- **Mods.** Two come with it: real battle sounds and sixteen new wilderness encounters. You can make your own (items, card texts, pictures, sounds, rules).

**You decide how much changes.** F12 > Settings > Preset offers presets, from *Full Original* (the original game's play exactly, bugs and all; the look stays yours to choose) and *Purist* (its bugs fixed) to *Darklands+* (recommended), and every switch one by one; easier play comes as built-in mods in F12 > Mods.

## Install

1. You need your own **Darklands 483.07** (GOG or Steam) and **64-bit Windows 10 or 11**. On Linux and Steam Deck it runs through Proton or Wine.
2. Download the zip from **[Releases](https://github.com/bouchacha/Darklands-plus/releases/latest)**, unpack it anywhere and run `darklands.exe`.
3. It finds your GOG or Steam installation by itself, or asks for the folder once. The first start takes a minute or two while Darklands+ prepares the game; after that it starts at once.

**From your Steam library:** in Steam, *Games > Add a Non-Steam Game to My Library*, *Browse* to `darklands.exe`, and add it.

**On a Steam Deck:** in Desktop Mode, unpack the zip into the Darklands folder (`steamapps/common/Darklands`), add `darklands.exe` as a non-Steam game as above, then in its *Properties > Compatibility* tick *Force the use of a specific Steam Play compatibility tool* and choose Proton. Back in Gaming Mode it is in your library, and the controller works.

## Questions

- **Do my saved games work?** Yes. They stay where they are, in the game's own format, and still load in the original game (unless a character uses one of the extra sprites Darklands+ offers). The first start of each version also copies them into a "Darklands+ save backups" folder.
- **Does it go online?** No. Darklands+ never connects to the internet and collects nothing.
- **My antivirus complains.** On the first start Darklands+ builds the game module, `darklands_game.dll`, from your own `DARKLAND.EXE`; some antivirus programs distrust files that were just compiled. Allow it.
- **Mac?** Not natively. Wine-based tools may work but are untested.
- **Something went wrong.** `darklands_port.log` in the game folder says why a start failed. See also `Darklands+ README.txt` in the download.
- **Where do I report bugs?** On GitHub, under [Issues](https://github.com/bouchacha/Darklands-plus/issues). Attach `darklands_port.log` from the game folder (`darklands_port_previous.log` if you have started the game again since) and, if you can, a save from just before the problem. The full list of features is in the [release notes](docs/release_notes.md), and [CHANGELOG.md](CHANGELOG.md) says what changed.

## Status

Actively developed. Bug reports, suggestions and contributions are welcome (see [CONTRIBUTING.md](CONTRIBUTING.md)). Yassine Meskhout directs and play-tests; the code is written by an AI (Anthropic's Claude). See [LICENSE](LICENSE).

## For developers

Darklands+ rests on a matching decompilation of Darklands 483.07: 382 of the game's source files are C that Microsoft C 6.00A compiles back to the original bytes, and `port/` recompiles the whole game to run natively. This repository holds no game files and no code generated from them. Start with [docs/DEVELOPERS.md](docs/DEVELOPERS.md), [CONTRIBUTING.md](CONTRIBUTING.md), [docs/MODDING.md](docs/MODDING.md) and [docs/MERGING.md](docs/MERGING.md).
