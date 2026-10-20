# What MicroProse left unused

Darklands keeps every card's text in `MSGFILES`: 419 cards. The game's code names 361 of them;
the other 58 are never loaded. Some card scripts are coded and still never reached, because
nothing in the game sends the party to them. This page lists what is there, how complete it
is, and what Darklands+ does with it. The [political campaign](cut_politics.md) has its own
page.

How it was found: every card name the executable can load was compared with the cards in
`MSGFILES`. Each unused text was then compared block by block with the cards the game does use,
which separates early drafts from content that exists nowhere else. For coded cards, the card
numbers the scripts jump to were checked against the arrival cards in `DARKLAND.LOC`, the map's
encounter tables and the other scripts.

## Restored

| | What MicroProse left | Darklands+ |
|---|---|---|
| **City politics** | 19 cards written and coded; the routine that starts a season is never called | Experimental option, off by default: *City politics* ([its page](cut_politics.md)) |
| **Cities at war** | 5 coded cards with 78 texts, plus effects across the game (prices, news, docks, a soldiers' street); the state that turns them on is never set, and the war's arrival card loads no text | Experimental option, off by default: *Cities at war* (below) |
| **"Fehde!"** | A finished wilderness encounter with no code | *Unused encounters*, on in the Darklands+ and Easy presets (below) |
| **Honor Roll of Glory** | An unfinished hall of fame | *Honor Roll of Glory* |

All of them are in F12 > Settings > Helpers > Cards and towns (the Honor Roll under Screens).
None is in the Buggy Purist preset.

## Cities at war

**How it was meant to play.** A city prepares for war, and later may be at war. The game reads
this state in many places:

- Prices go up by a quarter while the city prepares, and by half at war. Some prices for the
  party cost 8/3 as much at war.
- The news says so: people grumble about prices, and at war two citizens argue whether rats are
  better stewed or in a pie (`$Situa01`).
- Soldiers' Road opens in the arms-makers' quarter: an arms outfitter selling every kind of
  weapon. Go to lists it then. (A card for the street itself, "crowded with soldiers and
  camp-followers", with a leader the political campaign could win over, `$Soldi00` and
  `$Soldi01`, is coded but nothing leads to it.)
- At the docks, guard patrols must be dodged and fewer boats leave, at twice the price. At war
  the docks are barricaded, and the officer lets only "useless mouths" leave (`$Docks00`).
- No political season starts in a city at war.
- Arriving by ship leads to the war's own arrival card (card 0x73). It offers the gate by day
  or by night, rushing the walls by day, or sneaking up to them by night:
  - **At the gate**, guards turn away "casual visitors and wayfarers" and wave supply wagons
    through (`$CityG04`, 19 texts). The party can walk through as if it belonged, talk to the
    guards, pay a farmer to hide it in his hay, use an alchemical distraction, call on a
    saint, or fight.
  - **Rushing the walls** means "Spies trying to enter!", soldiers running after you and the
    wall looming overhead (`$CityW03`). Then: alchemy, a sally port to force (`$CityW05`), a
    saint, or a fight at the foot of the wall on a battlefield of its own.
  - **Sneaking up at night** brings the party to an old sally port (`$CityW04`): a rope, a
    climb, a sewer grating, alchemy on the door, a saint, or a bribe to the sergeant.
  - Surrendering ends in the dungeon (`$Dunge00`). A fight the party walks away from ends back
    in the wilderness, barred from the city and wanted by its watch.

**What was missing or broken.**

1. Nothing ever sets the state: byte +0x14 of a city's location record is 1 while it prepares
   for war and 2 at war. `DARKLAND.LOC` has 0 for every city, and the game's code writes only
   other bits.
2. Arriving by land always gives the ordinary arrival card. The location's arrival card was
   never switched to card 0x73, as the game does when a village is burnt down.
3. Card 0x73 sets its name to `$CityE00`, the ordinary arrival, and never loads it. Its menu was
   the previous card's, and the texts it shows (7 to 9) do not exist in `$CityE00`. Its code
   follows `$CityE01` block for block: a city that "seethes like an anthill" while "Clearly,
   the Frager is preparing for war". MicroProse wrote that text for this card and never used it.
4. `$CityG04` asks for a picture, "gate4", that was never drawn, and the game stops.
5. `$CityW03` and `$CityW04` gained "...leaving your horses behind" twins of some options after
   their code was written: 7 option lines for 5 handlers, and 10 for 7. From the first twin on,
   every line ran the next one's handler, so praying attacked and attacking ran away.
   `$CityW03` also shows its sally-port option, meant to be hidden, the first time.
6. The patrols at the walls fight as the game's "Brigand", though every text calls them
   soldiers and guards.
7. The arms-makers' quarter (`$MilCr00`) switches Soldiers' Road off, then switches it off
   again unless the city is at war, so the road never appears. Chosen, it would first show
   the quarter's own menu for the night or the fair as if that were a description.

**What Darklands+ does** (`rules.war`, port/mods/war.c):

1. Each quarter of the year, one city in twelve prepares for war for two to four weeks. Half of
   those then stand a siege of one to three weeks. The schedule comes from the date, the city
   and the party's seed, so nothing extra is saved. A city in a political season is left alone.
2. While it lasts, the city's arrival card is card 0x73.
3. Card 0x73 loads `$CityE01`.
4. `$CityG04` shows the city gate's picture (the one `$CityG01` uses).
5. The twins are taken out of `$CityW03`'s option text, and its hidden option stays hidden.
   `$CityW04` was already put right by the *cardopts* fix.
6. The patrols fight as the city's "Guard", an enemy of the same strength.
7. Soldiers' Road appears in the arms-makers' quarter while the war lasts, and leads straight
   to the arms outfitter.
8. The margin shows "Preparing for war" or "At war", with what that means.

Turning the option off puts every city back at peace. A save made during a war carries the war
into the original game, where card 0x73 is broken as described above.

**Tested** in Soest and Dortmund: arrival by land; the gate refusing wayfarers ("Soest is at
war. We have no need for useless mouths to feed"); rushing the walls, the chase and a fight at
the foot of the wall, back to the gate; sneaking up at night; the sewer grating; the bribe that
opens the sally port into the city; the news (rats); Soldiers' Road and its outfitter through
Go to; the margin. Not yet seen in play: the docks at war, and the dungeon after a surrender.

## "Fehde!"

`$MeetF00` (18 texts) is an ambush on the road by soldiers of a private war, with the battle
cry "Fehde!". Its texts follow the ambush by mercenaries turned bandits (`$MeetB02`, card 0x103)
block for block: the same menus, the same outcomes, even the same empty block 16. A few lines
are its own: "In a private war (feud), anyone not an ally is the enemy"; prisoners are paroled
on oath; "We defeated this group of soldiers, but how many more are roaming nearby?". It was
written for that card's code and never given a place in the encounter tables.

**What Darklands+ does** (`rules.cut_encounters`, port/mods/cutcards.c): half the times
`$MeetB02` comes up, it loads `$MeetF00` instead. Everything else is the card's own: the
chances, saints and potions, the battle and the losses. In the battle, the mercenaries are the
game's "Soldier" (the same enemy under another name), and the robber captain who leads them is
a knight, since declaring a feud was a knight's affair. The "Captain" of a stronger party stays.

## Written, never coded

These texts exist only in `MSGFILES`, with no code to show them.

- **The barber-surgeons' guild** (`$Barbe00`, `$Barbe01`: 19 and 18 texts). A guild street by
  day and by night, with the same options as the other guild streets of the political
  campaign: the shopfronts, the guild's leaders, revealing the guild master's secret, sneaking
  into his home. An early crafts-quarter card (`$CivCr01`) leads to the "Road of the Herbalists
  and Surgeon-Barbers". In the game as shipped, that line is the physician's townhouse.
- **The hospice** (`$CityH00`, 7 texts; `$CityH01`, a one-line placeholder for night). Give
  money, work a day and a night beside the nuns (Healing practice, and they pay you), have
  wounds healed for a fee, or beg to be healed for free. An early church-district card
  (`$Churc01`) leads to "the hospital". The shipped one has an empty line there, and the nuns'
  care went to the monastery (`$NunsH00`).
- **The annual fair at the gate** (`$CityE03`, 7 texts). The approach to a city crowded for
  its fair. The fair itself is in the game: within 14 days of a city's fair day, a shell-game
  man works the square and the market (`$Shell00`).
- **A second job poster** (`$DPost01`, 14 texts). "Tough crowd, not afraid to get their hands
  dirty": the poster leads to a moneylender's house. From its fifth text on, it is the
  warehouse task of `$DPost00`.
- **The execution, earlier** (`$Execu00`, 14 texts). A draft of the shipped `$Execu01`, with an
  outcome never written: "the city ruler asks you on a quest (UNFINISHED)".
- **The monastery, earlier** (`$Monas00`, `$Monas01`). Study the holy books for Religion and
  Latin, or pay the monks one to ten florins to chant psalms for you. It was replaced by the
  shipped monastery (`$CityM00`).
- **The political campaign's first investigation cards** (`$Situa00`, `$Noble00`, `$Guild02`,
  `$Forei01`, `$Speci01`, `$Guild01`, `$Inter00`). These sound out each group's leaning, and in
  one you "talk to the guild leader's mother about his father". They use `$VarPhrase`, which
  the engine never had.

## Drafts

These are earlier versions of cards the game uses:

- `$MineR00`–`$MineR05`: the mine where dwarfs and kobolds are at war, before it became
  `$MineR06`–`$MineR11` (the dwarfs' "volley of handguns" became thunderbolt potions).
- `$MineH02` is the same as `$MineH01`, and `$LastC12` as `$LastC11`.
- `$Witcd09` is nearly `$Witcd08`, and `$Villa04` is `$Villa00`.
- `$Raubi06`, the burnt-out keep, is `$Raubr00`.
- `$CityG02` is the war gate before `$CityG04`.

## Placeholders and jokes

- **Places never written**, mostly one line each: the barracks at night (`$CityB01`); the war
  gate at night (`$CityG03`, "This card (CityGateWarPrepNight) is not done. If only this menu
  did something."); a gunsmith (`$Gunsm00`, `$Gunsm01`); the imperial mint (`$Imper00`,
  `$Imper01`); notices and gossip in the square (`$Notic00`, `$Notic01`); the poorhouse
  (`$Poorh00`, `$Poorh01`); the slum at night (`$SlumN00`); the stables (`$Urban02`,
  `$Urban03`); the main streets (`$MainS00`, "the main 'going to places' menu"); a generic
  approach (`$Outsi00`).
- **Street hubs from an earlier city layout**: `$Churc01`, `$CivCr01`, `$MilCr01`, and
  `$SideS02` and `$SideS03` ("SIDE STREET A - ERROR!!!"). One line of these is "-- to be
  determined -- formerly the whorehouse".
- **Tests**: `$Strat01`–`$Strat04` ("this is a temp card"), `$Fight00`, and `$Overh01`
  ("Testing 1,2,3").
- **Jokes**: `$Rando01` meets "some horrible monster dredged from the slime churning in the
  bottom of Arnold and Sandy's depraved synapses". `$Destr00` answers a deleted card: "Do not
  pass GO. Do not collect $200."
