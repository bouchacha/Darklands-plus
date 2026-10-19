# Darklands+

**Darklands (MicroProse, 1992) running natively on Windows, without DOSBox, with the improvements players have wanted for thirty years. Every one of them can be switched off.**

<img width="1600" height="900" alt="A card drawn by Darklands+: crisp text, chances of success, the party and the town beside it" src="https://github.com/bouchacha/Darklands-plus/releases/download/1.2/1-cards-and-margins.png" />
<img width="1600" height="900" alt="Go to: every place in a city, with what it teaches, sells and offers" src="https://github.com/bouchacha/Darklands-plus/releases/download/1.2/2-city-go-to.png" />
<img width="1600" height="900" alt="A shop rebuilt for the mouse" src="https://github.com/bouchacha/Darklands-plus/releases/download/1.2/3-shop.png" />
<img width="1600" height="900" alt="The character sheet: skills, with the whole party compared" src="https://github.com/bouchacha/Darklands-plus/releases/download/1.2/4-character-sheet.png" />
<img width="1600" height="900" alt="A battle with its log and the enemies' state" src="https://github.com/bouchacha/Darklands-plus/releases/download/1.2/5-battle-log.png" />
<img width="1600" height="900" alt="The world map: find a place, see how far it is, travel there" src="https://github.com/bouchacha/Darklands-plus/releases/download/1.2/6-world-map-find.jpg" />
<img width="1600" height="900" alt="Making a character: every profession at once, with what it needs and teaches" src="https://github.com/bouchacha/Darklands-plus/releases/download/1.2/7-character-creation.png" />

## Play

1. You need your own **Darklands 483.07** (GOG or Steam) and **64-bit Windows 10 or 11**. On Linux and Steam Deck it runs through Proton or Wine.
2. Download the zip from **[Releases](https://github.com/bouchacha/Darklands-plus/releases/latest)**, unpack it anywhere and run `darklands.exe`.
3. It finds your GOG or Steam installation by itself, or asks for the folder once. The first start takes a minute or two. Your saved games stay where they are.

The download contains nothing from the game itself.

## What it adds

- **Modern screens** for the cards, shops, the character sheet, loot, the inn, alchemy, saints, saving and loading: crisp text at any window size, made for the mouse.
- **Less busywork:** the party's best member takes the tests the game puts to the leader, one pack serves the whole party, Go to walks you through a town, and the world map travels for you.
- **What the game kept hidden:** chances of success on card options, why an option is greyed, what saints and potions really do, and quests with deadlines and markers on the map.
- **Battles:** a log of every blow, fast-forward, better pathfinding, a click on a foe to attack it, enemy info and a map of the level.
- **38 fixes** for bugs of the original, and 39 saints that now do what their descriptions say.
- **The Roland MT-32 built in** (with your own ROM files), controllers and the Steam Deck, and mods. Two mods come with it: real battle sounds and sixteen new encounters.

F12 > Settings > Helpers has **presets**, from *Buggy Purist* (the 1992 game exactly) and *Purist* (its bugs fixed) to *Easy*. The full list is in the [release notes](docs/release_notes.md); [CHANGELOG.md](CHANGELOG.md) has what is new.

## Status

Finished and handed over. The author is not a programmer and will not maintain the project or answer bug reports. The code was written by an AI (Anthropic's Claude) from the author's direction and play-testing, and anyone is free to fork it and carry it on. See [LICENSE](LICENSE).

## Under the hood

Darklands+ rests on a matching decompilation of Darklands 483.07: 382 of the game's source files are C that Microsoft C 6.00A compiles back to the original bytes, and `port/` recompiles the whole game to run natively. This repository holds no game files and no code generated from them. Start with [docs/DEVELOPERS.md](docs/DEVELOPERS.md), [CONTRIBUTING.md](CONTRIBUTING.md), [docs/MODDING.md](docs/MODDING.md) and [docs/MERGING.md](docs/MERGING.md).
