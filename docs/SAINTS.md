# Saints: what the code does, what the texts say, what Darklands+ changes

Each saint's help is a function in the overlay s039 (`src/c/s039/s039_165c.c`), called through the
table `saint_fns` (far 290e:2937) with `what` 8. Saint *i* (the order of DARKLAND.LST) is the
function vector *i* leads to, not the *i*-th in the file. The same function answers `what` 0 to 6
with the saint's table (feast day, kind, the divine favour to spend at least and at most, the
virtue needed, the base chance) and `what` 7 with the chance:
(virtue − needed) / 2 + base + favour spent − the least favour, 0 below the virtue or the least favour.

What the help does, in the code:

- **Boosts for about a day** (19 to 21 hours: `raise_attr_temp`, s029 0e76:4c04, attributes; its
  twin 4b08, skills). Negative ones are the price some saints ask. "The skills of the weapons in
  hand" (0e76:0792) raises the skill of each kind of weapon the member has ready (melee and missile).
- **Healing** (`heal_attr_pct`): a part of what is lost of Endurance and Strength.
- **Reputation** (`loc_reputation_adj`, 0e76:19d0): by *a* to *b* − 1, with a chance of 100 less
  how far the town's reputation already is from 0; "the nearest town" when the party is in none.
- **Events**: kind 0x4b for a day, water crossed as if it were land (s030, the map); 0x4f for a
  week, no ambush in the meetings on the road (`$MeetB01` and three more cards); 0x5d, the next
  step on the map leads to the nearest town.
- Everything else the descriptions promise (banishing demons, escaping prison, light, sensing evil
  ...) happens only where an encounter's card lets a member call on the saint: the scripts list the
  saints they accept (`card_saints`). The last column counts them: many (10 or more scripts), some
  (3 to 9), a few, none.

Darklands+ shows all of this beside each description (F12 > Saints, the character sheet, the
prayer screen, the margins): the description in ink, **What it does** (below) in blue, and
**Unlike the text** where the description promises what the code does not
(`port/mods/saintinfo.c`). The switches are in F12 > Settings > Helpers > Fixes.

## Fixed

- **pauls**: the table sends St. Paul (105) to the help the texts and the manual give St. Paul the
  Simple (165c:5cca: all Strength and Endurance healed, half the Intelligence lost; virtue 70) and
  St. Paul the Simple (106) to a copy of St. Thomas Aquinas's help (5d6c). The Apostle's own
  function (75d8: Speak Common, Latin, Read & Write, Charisma, Endurance, a little Strength
  healed; virtue 19, as the manual) is in the game but nothing calls it. Now each Paul does what
  his text says. Players knew it as "the Pauls are swapped".
- **saintrep**: St. John Nepomuk in Prag passes an uninitialised local as the place, and St.
  Cecilia on the map passes −1 (the other saints take the nearest town): reputation written to a
  random town or to memory that is not a place's. Now Prag and the nearest town get it.
- **armourday** (with the potions): St. Dunstan's +1 and St. Patrick's +2 armour never wore off and
  piled up; St. Crispin added 10 to the quality of every member's worn armour, for good and piling
  up, which also broke the link between the armour in use and its record in the pack (the game
  knows the worn armour by its type and quality). St. Polycarp gave every member fire protection
  90 for good, and he, St. Charity, St. Erasmus and St. Pantaleon wrote their protection against
  fire into the bits of the armour byte (party +0x41) that the battle counts as armour: +32 or +48
  armour for good (the fireward fix hides those bits, so with it they gave nothing at all). Now
  each lasts a day, the fire saints as fire protection 90 (Polycarp's) for the one prayed for
  (Polycarp: everyone).
- **battlesaints** (earlier): help called in a battle never wore off.

## Left as the game has it

The descriptions promise things the code never had; Darklands+ says so beside them and changes
nothing (the game's balance is the code's):

- **Faster travel**: St. Christopher, St. Drogo, St. Wilfrid ("may double party travel speed") and
  St. Eustace (through woods). No code anywhere changes the party's speed; Drogo and Wilfrid give a
  week safe from ambushes instead, Christopher and Eustace nothing on the map. (Players reported it:
  "neither do any of the saints who are said to increase the party's speed".)
- **Reputation in lands that are not in the game**: Andrew (Scotland, Russia), Denis (Paris,
  France), Devota (Corsica, Monaco), Eric (Sweden), Isidore (Madrid, Spain), James (Spain), Paul
  (Greece, Malta), Stanislaus (Poland, Cracow). The manual itself writes "distant lands".
- **Reputation the help does not give** (it may come in encounters): Hildegard, Henry, Raymond
  Penafort (the Dominicans), Thomas Aquinas (monasteries and universities), Willibrord, Wolfgang,
  Wenceslaus (with nobles).
- **Reputation the texts limit**: Nicholas raises it in any town (the text: his lands); Florian only
  in the Austrian towns (the text: Poland too); Hedwig in Frankfurt an der Oder, Breslau, Olmütz
  and Teschen; Gottschalk in exactly the seven towns the text names; Michael anywhere (the text:
  "Germany and Brussels").
- **Other differences**: Nicholas and Theodore Tiro give Flails where the texts say Riding (Nicholas
  also no Agility), Charity Flails and reputation her text does not mention; Thomas Aquinas no
  Religion (Speak Common and Healing instead); Francis no Virtue and no loss of money (Healing,
  Artifice and Woodwise instead); Giles a little Woodwise and a tremendous Streetwise (the text: the
  other way round) and no money; Dismas only the one prayed for, no Charisma or Speak Common;
  Eustace only Edged, Thrown and Bows; Dunstan armour one level thicker for the one prayed for
  (the text: the quality of the party's metal armour), Crispin the quality of all armour (the text:
  non-metal); Patrick's armour against every blow (the text: missiles); Michael's "weapons and
  armour in combat" are the skills of the weapons in hand; Pantaleon water crossing and no help
  against animals, Margaret no protection from flames; Lutgardis raises Endurance (the text: it
  costs Endurance), Vitus and James cost nothing; Januarius has no help of his own (his beasts are
  the encounters').

## The table

Virtue: the least needed. Favour: the divine favour to spend (least to most). Base: the base
chance. What it does: with the fixes on. Encounters: how many card scripts accept the saint.

| # | Saint | Virtue | Favour | Base | What it does | Unlike the text / fixed | Encounters |
|---|---|---|---|---|---|---|---|
| 0 | St. Adrian | 43 | 15-99 | 25% | For a day: Endurance +8 to 15, Charisma +6 to 11, every weapon skill +15 to 29. |  | many |
| 1 | St. Agatha | 24 | 20-80 | 25% | For a day: Endurance +6 to 11 (a woman: +12 to 23), Charisma +6 to 11. |  | none |
| 2 | St. Agnes | 19 | 10-99 | 25% | Only if a woman is in the party: may raise the reputation in the nearest town by 10 to 19; for a day, Charisma +10 to 29. |  | a few |
| 3 | St. Aidan | 23 | 15-99 | 25% | For a day: Woodwise +25 to 29. |  | many |
| 4 | St. Albert the Great | 39 | 25-99 | 60% | For a day: Intelligence +8 to 15, Perception +6 to 11, Alchemy +30 to 44, Latin and Read & Write +15 to 29. |  | none |
| 5 | St. Alcuin | 44 | 15-99 | 65% | For a day: Intelligence +6 to 11, Latin and Read & Write +20 to 39. |  | some |
| 6 | St. Alexis | 28 | 10-99 | 40% | May raise the reputation in the nearest town by 10 to 19. For a day: Charisma +6 to 11. A party with neither florins nor groschen gets 3 to 9 groschen. |  | a few |
| 7 | St. Andrew | 14 | 5-50 | 20% | For a day: Endurance +6 to 11, Charisma +8 to 15. | The reputation in Scotland or Russia: places not in the game. | none |
| 8 | St. Andrew the Tribune | 37 | 20-99 | 15% | For a day: Edged, Impact, Polearms, Thrown and Bows +15 to 29. |  | none |
| 9 | St. Anthony | 35 | 20-99 | 35% | For a day: Endurance +4 to 9, Strength +5 to 10, Charisma +1 to 10, Speak Common and Latin +10 to 19, Read & Write +20 to 39. |  | some |
| 10 | St. Anthony of Padua | 59 | 10-99 | 65% | For a day: Perception +6 to 11, Strength +4 to 7, Charisma +15 to 24, Speak Common +20 to 29, Impact and Latin +10 to 19. |  | a few |
| 11 | St. Arnulf | 18 | 15-99 | 25% | For a day: Perception +6 to 11, Strength and Intelligence +4 to 7, every weapon skill +10 to 19, Streetwise and Latin +6 to 11, Read & Write and Riding +8 to 15. |  | none |
| 12 | St. Apollinarius | 78 | 20-50 | 70% | For a day: Endurance +8 to 23, Strength +2 to 4. |  | some |
| 13 | St. Barbara | 32 | 20-50 | 40% | Artifice +15 to 29 for two hours at most; Missile Devices +20 to 39 for 5 to 7 hours. |  | a few |
| 14 | St. Bathildis | 21 | 15-60 | 20% | For a day: Intelligence +4 to 8, Perception +3 to 6. |  | a few |
| 15 | St. Boniface | 25 | 10-90 | 10% | For a day: Charisma +8 to 15, every weapon skill +10 to 19, Speak Common +8 to 15. |  | many |
| 16 | St. Catherine | 46 | 30-70 | 30% | For a day: Charisma and Intelligence +8 to 15, Speak Common +40 to 99. |  | a few |
| 17 | St. Catherine of Siena | 33 | 10-99 | 25% | For a day: Charisma and Perception +1 to 15, Speak Common +10 to 29. |  | many |
| 18 | St. Cecilia | 41 | 25-99 | 50% | For a day: Charisma +3 to 10. If the one prayed for carries a musical instrument, it may raise the reputation of the town the party is in (on the map: the nearest) by 20 to 59. | **Fixed** (saintrep): on the map (in no town) the game wrote the reputation to "location -1", memory before the list of places. | some |
| 19 | St. Ceolwulf | 23 | 15-99 | 25% | For a day: Perception and Intelligence +3 to 5, Strength +6 to 11, every weapon skill and Latin +6 to 11. |  | none |
| 20 | St. Charity | 40 | 10-50 | 50% | Fire protection 90 for a day, for the one prayed for. May raise the reputation in the nearest town by 10 to 19. For a day: Charisma +6 to 11, Flails +15 to 29. | Not in the text: Flails and the reputation. The fire protection is for the one prayed for only (the text: everyone, and one person greatly), and no Endurance is healed. **Fixed** (armourday): the game wrote its fire protection where the battle reads armour: +32 armour for good (nothing with the fireward fix). | some |
| 21 | St. Christina, Astonish. | 54 | 50-99 | 50% | For a day: Agility +8 to 15, Charisma +1 to 10. |  | some |
| 22 | St. Christopher | 31 | 15-99 | 25% | For a day: Streetwise +10 to 19, Woodwise +20 to 39, Riding +25 to 74, Bows +6 to 11. | Not in the game: faster travel, and the Strength and Endurance restored. | a few |
| 23 | St. Clare | 33 | 30-70 | 20% | Heals all lost Endurance and Strength. |  | some |
| 24 | St. Clotilda | 22 | 10-99 | 30% | Heals 25% of lost Endurance and 50% of lost Strength. For a day: Charisma +6 to 11, Healing +10 to 19. |  | none |
| 25 | St. Colman of Cloyne | 34 | 15-99 | 55% | For a day: Charisma +8 to 15, Speak Common +30 to 89, Latin +6 to 11, Read & Write +10 to 19. |  | none |
| 26 | St. Columba | 36 | 20-99 | 60% | For a day: Endurance +8 to 15, Strength +6 to 13, Charisma +4 to 7, Perception -5 to 9, the skills of the weapons in hand +10 to 19. |  | none |
| 27 | St. Cosmas | 15 | 15-75 | 30% | Heals 10% of lost Endurance and 10% of lost Strength. For a day: Perception +4 to 7, Healing +40 to 79. Teaches St. Damian. |  | none |
| 28 | St. Crispin | 28 | 25-99 | 35% | Every member's worn armour counts 10 points better in quality, for a day. | The text says non-metal armour; the game raises all armour. **Fixed** (armourday): the game added 10 to the quality of every member's worn armour for good, piling up, and the armour in use then no longer matched its record in the pack. | none |
| 29 | St. Cyprian | 54 | 25-99 | 50% | For a day: Intelligence +5 to 9, Alchemy +5 to 9. |  | some |
| 30 | St. Damian | 30 | 25-80 | 40% | Heals all lost Endurance and 30% of lost Strength. Teaches St. Cosmas. |  | none |
| 31 | St. David | 30 | 10-75 | 25% | For a day: Intelligence +4 to 7, Charisma +1 to 6, Perception +6 to 11, the skills of the weapons in hand +8 to 15, Riding +6 to 11. |  | none |
| 32 | St. Denis | 38 | 25-99 | 20% | Heals all lost Strength. For a day: Charisma +15 to 44, Speak Common +25 to 49. | The reputation in Paris and France: places not in the game. | none |
| 33 | St. Derfel Gadarn | 57 | 20-60 | 45% | Heals 30% of lost Endurance. For a day: Strength +5 to 14, the skills of the weapons in hand +20 to 59. |  | none |
| 34 | St. Devota | 26 | 20-65 | 10% | Heals all lost Endurance and 20% of lost Strength. For a day: Charisma +15 to 29. | The reputation in Corsica and Monaco: places not in the game. | some |
| 35 | St. Dismas | 53 | 30-99 | 45% | For a day: Agility +10 to 19, Woodwise and Stealth +10 to 19, Streetwise and Artifice +20 to 39. | Only the one prayed for, not the entire party; no Charisma or Speak Common. | some |
| 36 | St. Dominic | 29 | 20-99 | 15% | For a day: Intelligence +10 to 19, Speak Common and Latin +10 to 19, Read & Write +20 to 39. |  | some |
| 37 | St. Dorothy of Montau | 52 | 20-99 | 50% | For a day: Charisma +10 to 19, Perception +6 to 11, Healing +10 to 19. A member with no edged weapon gets a long sword. |  | many |
| 38 | St. Drogo | 59 | 15-99 | 60% | For a day: Perception +6 to 11, Healing +10 to 19, Woodwise +20 to 39. For a week no ambush takes the party by surprise (bandits and the like). | Not in the game: faster travel. Instead, a week safe from ambushes. | a few |
| 39 | St. Dunstan | 45 | 30-60 | 60% | For a day: Perception +6 to 11, the skills of the weapons in hand +6 to 11, Artifice +25 to 74, Streetwise +6 to 11, Alchemy +5 to 14. Armour one level thicker, for a day. | The text says the quality of all metal armour in the party; the game gives the one prayed for armour one level thicker. **Fixed** (armourday): the game gave +1 armour for good, piling up. | none |
| 40 | St. Dymphna | 21 | 20-75 | 10% | Heals 30% of lost Endurance. For a day: Agility +4 to 7, Stealth +15 to 29, Healing +10 to 19. |  | a few |
| 41 | St. Edward, Confessor | 5 | 20-99 | 10% | For a day: Endurance +4 to 7, Intelligence +6 to 11, Perception +8 to 15, Riding +8 to 15, every weapon skill +6 to 11. Every member's Virtue is raised to at least 20, for good, and the party loses a quarter of its money. |  | some |
| 42 | St. Eligius | 29 | 10-60 | 25% | For a day: Artifice +25 to 74, Alchemy +8 to 15. |  | none |
| 43 | St. Emydius | 31 | 20-99 | 55% | For a day: Charisma +15 to 29, Speak Common +20 to 39. |  | some |
| 44 | St. Engelbert | 40 | 15-99 | 30% | For a day: Charisma +5 to 9, Riding and Woodwise +10 to 19, Speak Common +6 to 11. For a day the party can cross water as if it were land. |  | a few |
| 45 | St. Erasmus | 32 | 15-50 | 40% | Fire protection 90 for a day, for the one prayed for. For a day: Agility and Perception +6 to 11. | **Fixed** (armourday): the game wrote its fire protection where the battle reads armour: +32 armour for good (nothing with the fireward fix). | a few |
| 46 | St. Eric | 42 | 20-99 | 25% | For a day: Strength +8 to 23, Charisma +4 to 7, the skills of the weapons in hand +15 to 24, Riding and Speak Common +6 to 11. | The reputation in Sweden: a place not in the game. | some |
| 47 | St. Eustace | 41 | 30-99 | 30% | For a day: Riding +10 to 19, Woodwise +40 to 69, Edged, Thrown and Bows +6 to 11. | Not in the game: faster travel through woods. The weapon skills are only Edged, Thrown and Bows. | a few |
| 48 | St. Felix of Nola | 37 | 25-99 | 30% | For a day: Agility +8 to 15, Stealth +40 to 119, Woodwise +10 to 19. |  | a few |
| 49 | St. Finbar | 17 | 30-99 | 45% | For a day: Charisma +8 to 15, Perception +6 to 11, Latin +10 to 19. |  | a few |
| 50 | St. Finnian | 24 | 40-99 | 35% | For a day: Agility +8 to 15, Charisma +6 to 11, Streetwise +4 to 7, Speak Common and Read & Write +6 to 11. For a day the party can cross water as if it were land. |  | some |
| 51 | St. Florian | 17 | 25-99 | 35% | May raise the reputation by 15 to 24 in Passau, Salzburg, Linz, Steyr, Wien or Graz (nowhere else). For a day: the skills of the weapons in hand +6 to 11, Riding +6 to 11. For a day the party can cross water as if it were land. |  | some |
| 52 | St. Francis | 10 | 10-60 | 1% | Heals 30% of lost Endurance and 10% of lost Strength. For a day: Perception and Endurance +6 to 11, Charisma +15 to 29, Speak Common and Woodwise +4 to 7, Healing +10 to 19, Artifice +12 to 23. | Not in the game: the Virtue and the loss of cash. Instead Healing, Artifice and Woodwise. | a few |
| 53 | St. Gabriel | 56 | 30-75 | 45% | For a day: Intelligence and Perception +10 to 29. |  | many |
| 54 | St. Genevieve | 73 | 40-99 | 75% | Heals 30% of lost Endurance and 20% of lost Strength. For a week no ambush takes the party by surprise (bandits and the like). |  | many |
| 55 | St. George | 23 | 40-99 | 10% | For a day: Strength +10 to 14, Agility +5 to 9, every weapon skill and Riding +25 to 49. |  | none |
| 56 | St. Gerlac | 34 | 40-75 | 50% | Heals 30% of lost Endurance and 20% of lost Strength. For a day: the skills of the weapons in hand +15 to 24, Healing +20 to 39. |  | none |
| 57 | St. Gertrude o'Nivelles | 26 | 35-99 | 30% | For a day: Charisma +6 to 11, Riding +5 to 9, Woodwise, Latin and Read & Write +15 to 29. At the party's next step on the map it is carried to the nearest town. |  | some |
| 58 | St. Giles | 45 | 35-99 | 15% | Heals 40% of lost Endurance and 10% of lost Strength. For a day: Agility -4 to 7, the skills of the weapons in hand -15 to 24, Stealth +30 to 59, Woodwise +5 to 9, Streetwise +50 to 89. | Woodwise and Streetwise the other way round (the text: a little Streetwise, a tremendous Woodwise); no money. | none |
| 59 | St. Giles of Portugal | 64 | 15-99 | 60% | For a day: Intelligence +5 to 9, Perception +15 to 24, Alchemy +40 to 59, Artifice +10 to 19. Costs a point of Strength (a wound) and a point of Virtue, for good. |  | none |
| 60 | St. Godehard | 39 | 15-99 | 55% | Heals 30% of lost Endurance. For a day: Latin +20 to 39, Read & Write +25 to 49. |  | some |
| 61 | St. Godfrey | 55 | 25-99 | 40% | Heals 20% of lost Endurance. For a day: Speak Common +15 to 29, Artifice +10 to 19. For a week no ambush takes the party by surprise (bandits and the like). |  | many |
| 62 | St. Gottschalk | 44 | 25-50 | 55% | May raise the reputation by 20 to 39 in Schleswig, Lübeck, Wismar, Rostock, Hamburg, Lüneburg or Bremen (nowhere else). For a day: every weapon skill +15 to 29. |  | none |
| 63 | St. Gregory Thaumaturg. | 74 | 30-99 | 80% | For a day: Charisma +8 to 15, Alchemy and Speak Common +10 to 19, Artifice +20 to 39. |  | many |
| 64 | St. Hedwig | 16 | 15-75 | 10% | May raise the reputation by 30 to 49 in Frankfurt an der Oder, Breslau, Olmütz or Teschen (nowhere else). For a day: Healing +20 to 29. |  | some |
| 65 | St. Henry | 61 | 30-99 | 75% | For a day: Intelligence +8 to 23, every weapon skill +10 to 19. | No reputation in its help. | a few |
| 66 | St. Heribert | 30 | 20-99 | 35% | For a day: Charisma +10 to 19, Strength +4 to 7. |  | some |
| 67 | St. Herve | 47 | 15-99 | 40% | For a day: Perception +15 to 29. |  | many |
| 68 | St. Hildegard | 13 | 25-99 | 25% | For a day: Perception +20 to 59. | No reputation in its help. | many |
| 69 | St. Hubert | 34 | 50-99 | 50% | For a day: Polearms, Thrown and Bows +15 to 29, Missile Devices +10 to 19, Stealth +25 to 49, Woodwise +40 to 79. |  | many |
| 70 | St. Illtyd | 63 | 20-99 | 65% | For a day: Charisma +15 to 29, every weapon skill and Speak Common +10 to 19, Riding +5 to 9. |  | none |
| 71 | St. Isidore | 21 | 10-99 | 35% | For a day: Charisma +8 to 15, Streetwise +10 to 19, Speak Common +5 to 9. | The reputation in Madrid and Spain: places not in the game. | a few |
| 72 | St. Ita | 85 | 10-99 | 70% | Heals 50% of lost Endurance and all lost Strength. For a day: Charisma +8 to 15, Healing +20 to 39. |  | none |
| 73 | St. James | 25 | 12-50 | 40% | Heals 20% of lost Endurance and 10% of lost Strength. For a day: Strength and Endurance +4 to 7, Charisma +3 to 5, Virtue +20 to 34. | The reputation in Spain: a place not in the game; no Intelligence lost. | none |
| 74 | St. Januarius | 66 | 35-75 | 50% | No help of its own: its powers work only where an encounter calls for it. |  | some |
| 75 | St. John of Bridlington | 36 | 10-99 | 15% | A woman: heals all lost Endurance and 20% of lost Strength, and for a day Endurance +5 to 9, Strength +4 to 7. A man: heals 30% of lost Endurance and 10% of lost Strength. |  | none |
| 76 | St. John Chrysostom | 37 | 10-50 | 25% | For a day: Charisma +8 to 15, Perception -6 to 11, Speak Common and Latin +30 to 59. |  | none |
| 77 | St. John Climacus | 20 | 40-99 | 30% | For a day: Alchemy, Religion, Speak Common, Latin, Read & Write, Healing, Artifice, Stealth, Streetwise and Woodwise each +1 to 4, and 1 more for every 5 points of Virtue. |  | none |
| 78 | St. John Nepolmuchen | 22 | 15-60 | 30% | May raise the reputation by 15 to 24 in Prag, and by 10 to 19 in St. Joachimsthal, Bürglitz, Kuttenberg, Olmütz or Brünn (nowhere else). For a day: Charisma +8 to 15, Intelligence +6 to 11, Speak Common +20 to 39, Streetwise +4 to 7. | **Fixed** (saintrep): in Prag the game sent the reputation to a place read from an uninitialised local. | some |
| 79 | St. Joseph | 27 | 35-99 | 35% | For a day: Charisma +6 to 11, Artifice and Streetwise +30 to 59. |  | a few |
| 80 | St. Jude | 15 | 20-90 | 5% | For a day: Endurance, Charisma, Strength, Perception, Agility and Intelligence +4 to 8, the skills of the weapons in hand +15 to 24, Alchemy, Religion, Speak Common, Latin, Read & Write, Healing, Stealth, Artifice, Woodwise and Streetwise +6 to 11. |  | some |
| 81 | St. Julian, Hospitaler | 61 | 40-99 | 75% | For a day: Woodwise +20 to 39, Riding +6 to 11. For a day the party can cross water as if it were land. |  | a few |
| 82 | St. Kessog | 31 | 20-99 | 25% | Heals 40% of lost Endurance and 20% of lost Strength. For a day: Charisma +6 to 11, Speak Common +15 to 29, Riding +4 to 7, Healing +10 to 19. |  | none |
| 83 | St. Lasdislaus | 51 | 25-99 | 60% | For a day: Strength +4 to 7, Agility +3 to 5, the skills of the weapons in hand +15 to 24, Speak Common +6 to 11, Riding +10 to 19. |  | none |
| 84 | St. Lawrence | 20 | 30-99 | 10% | Heals all lost Endurance. For a day: Charisma +12 to 19, Speak Common and Streetwise +10 to 19. |  | some |
| 85 | St. Lazarus | 80 | 50-99 | 60% | Heals 30% of lost Endurance and all lost Strength. |  | a few |
| 86 | St. Longinius | 49 | 30-99 | 45% | For a day: every weapon skill +10 to 19. |  | none |
| 87 | St. Lucy | 77 | 40-99 | 65% | Heals all lost Endurance and Strength. For a day: Perception +6 to 11. |  | some |
| 88 | St. Luke | 17 | 20-99 | 35% | Heals 50% of lost Endurance and 30% of lost Strength. For a day: Healing +20 to 39. |  | none |
| 89 | St. Lutgardis | 60 | 35-99 | 50% | For a day: Charisma +6 to 11, Perception +8 to 15, Endurance +5 to 9, Virtue +6 to 17. | Endurance is raised, not spent. | many |
| 90 | St. Margaret | 46 | 25-75 | 45% | A woman: heals all lost Endurance and half the lost Strength, and for a day every attribute +3 to 6 and Alchemy, Religion, Speak Common, Latin, Read & Write, Healing, Artifice, Stealth, Streetwise and Woodwise +5 to 10. A man: heals 10% of lost Endurance and Strength. | Not in the game: the protection from flames. | some |
| 91 | St. Margaret of Cortona | 52 | 25-99 | 10% | Heals all lost Endurance and 40% of lost Strength. For a day: Healing +20 to 39, Speak Common +10 to 19. |  | none |
| 92 | St. Mark | 35 | 25-99 | 40% | For a day: Agility +12 to 23, Read & Write +15 to 29. |  | none |
| 93 | St. Martin of Tours | 69 | 25-99 | 65% | Heals 30% of lost Endurance and 30% of lost Strength. For a day: Charisma +8 to 15, Perception +12 to 19, Speak Common +25 to 49, Riding +20 to 39. |  | many |
| 94 | St. Matthew | 26 | 20-80 | 25% | For a day: Intelligence +8 to 15, Latin +15 to 29, Read & Write +25 to 49. The party's standing with the Fugger and with the Medici rises by 20 to 39 each. |  | none |
| 95 | St. Maurice | 81 | 30-50 | 70% | For a day: Edged +20 to 39, Alchemy +10 to 19. |  | none |
| 96 | St. Michael | 72 | 25-99 | 55% | May raise the reputation in the nearest town by 20 to 39. For a day: Charisma +12 to 19, the skills of the weapons in hand +20, Healing +20 to 39. | In combat it raises the skills of the weapons in hand, not the weapons and armour. | none |
| 97 | St. Milburga | 71 | 35-99 | 70% | For a day: Charisma +10 to 19, Healing +20 to 39. For a day the party can cross water as if it were land. |  | many |
| 98 | St. Moses the Black | 66 | 10-99 | 65% | For a day: every weapon skill +15 to 29, Stealth +20 to 39, Woodwise and Streetwise +10 to 19. |  | a few |
| 99 | St. Nicholas | 49 | 25-99 | 55% | May raise the reputation in the nearest town by 10 to 19. For a day: Charisma +6 to 11, Endurance +4 to 8, Flails +15 to 29. | Flails and Endurance instead of Agility and Riding; the reputation anywhere, not only in his lands. | a few |
| 100 | St. Odilia | 25 | 20-99 | 40% | May raise the reputation by 15 to 24 in Strassburg or Basel (nowhere else). For a day: Perception +6 to 11. |  | a few |
| 101 | St. Odo | 47 | 30-99 | 60% | For a day: Perception +10 to 19, Charisma +4 to 8, every weapon skill +8 to 22, Speak Common +8 to 15. |  | none |
| 102 | St. Olaf | 18 | 15-99 | 20% | Heals 30% of lost Endurance. For a day: every weapon skill +8 to 22. |  | none |
| 103 | St. Pantaleon | 56 | 25-99 | 20% | Fire protection 90 for a day, for the one prayed for. Heals 30% of lost Endurance and 30% of lost Strength. For a day: Charisma +6 to 11, Alchemy +10 to 19, Healing +15 to 29. For a day the party can cross water as if it were land. | Fire protection for the one prayed for (not the party), and the crossing of water, not in the text; nothing against animals. **Fixed** (armourday): the game wrote its fire protection where the battle reads armour: +32 armour for good (nothing with the fireward fix). | some |
| 104 | St. Patrick | 22 | 25-99 | 15% | For a day: Charisma +15 to 24, Strength +12 to 19, Edged, Impact, Polearms, Speak Common, Latin and Read & Write +15 to 29. Armour two levels thicker, for a day. | The armour is against every blow, not only missiles. **Fixed** (armourday): the game gave +2 armour for good, piling up. | none |
| 105 | St. Paul | 19 | 27-52 | 35% | Heals 10% of lost Strength. For a day: Endurance +5 to 9, Charisma +8 to 15, Speak Common +20 to 39, Latin +10 to 19, Read & Write +40 to 79. | The reputation in Greece and Malta: places not in the game. **Fixed** (pauls): the game gave Paul the Simple's help (and his virtue 70). | none |
| 106 | St. Paul the Simple | 70 | 20-80 | 60% | Heals all lost Endurance and Strength. For a day: Intelligence lowered by half its maximum. | **Fixed** (pauls): the game gave a copy of St. Thomas Aquinas's help (virtue 19). | some |
| 107 | St. Perpetua | 16 | 40-99 | 10% | For a day: Charisma +15 to 29. |  | some |
| 108 | St. Peter | 58 | 25-75 | 55% | For a day: Charisma +8 to 15, Strength +12 to 19, Perception lowered by half its maximum, Speak Common, Latin and Healing +10 to 19, Edged +15 to 29. |  | some |
| 109 | St. Peter of Atroa | 62 | 25-99 | 50% | May lower the reputation in the nearest town by 1 to 4. For a day: Intelligence and Perception +6 to 11, Alchemy and Riding +10 to 19, Stealth +40 to 79. |  | none |
| 110 | St. Polycarp | 19 | 10-75 | 20% | Every member: fire protection 90 for a day. | **Fixed** (armourday): the game gave every member fire protection 90 for good, and +48 armour for good (nothing with the fireward fix). | some |
| 111 | St. Raphael | 75 | 40-60 | 70% | Heals all lost Endurance and Strength. For a day: Agility +8 to 15, Perception +6 to 11, Healing +50 to 139. |  | many |
| 112 | St. Raymond Penafort | 67 | 10-99 | 50% | For a day: Intelligence +15 to 29, Read & Write +30 to 89. For a day the party can cross water as if it were land. | No reputation with the Dominicans in its help. | some |
| 113 | St. Raymond Lull | 29 | 10-99 | 35% | For a day: Intelligence +12 to 19, Alchemy +20 to 39, Read & Write +30 to 59. Endurance and Strength below 9 are raised to 9. |  | none |
| 114 | St. Reinold | 27 | 25-99 | 45% | Heals 30% of lost Endurance. For a day: Charisma -6 to 11, Alchemy +6 to 17, Artifice +35 to 69. |  | some |
| 115 | St. Roch | 19 | 40-90 | 45% | Heals 20% of lost Endurance and 20% of lost Strength. For a day: Healing +10 to 19, Artifice +8 to 15. |  | a few |
| 116 | St. Sabas the Goth | 64 | 25-99 | 50% | Heals all lost Endurance and Strength. For a day: Charisma +10 to 19. |  | none |
| 117 | St. Sebastian | 28 | 15-90 | 10% | Heals 10% of lost Endurance and 10% of lost Strength. For a day: Agility +12 to 19, Bows +20 to 39, Riding +10 to 19. |  | a few |
| 118 | St. Stanislaus | 42 | 40-99 | 65% | Heals 20% of lost Endurance and 10% of lost Strength. For a day: Charisma +6 to 11, Speak Common +30 to 59, Latin +20 to 39. | No reputation in its help (Poland and Cracow: no such places in the game). | none |
| 119 | St. Stephen | 27 | 20-99 | 30% | In Pressburg, may raise the reputation by 15 to 24. For a day: Charisma +6 to 11, Intelligence +8 to 15, the skills of the weapons in hand +15 to 24, Speak Common +6 to 11, Riding +10 to 19. |  | none |
| 120 | St. Swithbert | 25 | 30-99 | 65% | Heals 20% of lost Endurance and 10% of lost Strength. For a day: Charisma +10 to 19, Speak Common +25 to 49. |  | none |
| 121 | St. Tarachus | 50 | 40-99 | 30% | Heals 40% of lost Endurance and 20% of lost Strength. For a day: Charisma +6 to 11. |  | some |
| 122 | St. Thalelaeus,Merciful | 62 | 10-99 | 65% | Heals 40% of lost Endurance and 20% of lost Strength. For a day: Healing +30 to 59. |  | none |
| 123 | St. Theodore Tiro | 38 | 25-99 | 40% | For a day: Charisma +6 to 11, Endurance +4 to 8, the skills of the weapons in hand +15 to 24, Flails +15 to 29. | Flails instead of Riding. | none |
| 124 | St. Thomas the Apostle | 24 | 10-65 | 15% | For a day: Strength and Endurance +4 to 8, Speak Common and Woodwise +4 to 7, Healing +10 to 19, Artifice +12 to 23. |  | none |
| 125 | St. Thomas Aquinas | 68 | 15-99 | 65% | For a day: Intelligence +12 to 23, Speak Common and Latin +12 to 26, Read & Write +15 to 29, Healing +10 to 19. | Not in the game: Religion and the reputation at monasteries and universities. Instead Speak Common and Healing. | some |
| 126 | St. Valentine | 48 | 10-50 | 45% | Heals 10% of lost Endurance and 10% of lost Strength. For a day: Charisma +10 to 19, Stealth, Healing and Speak Common +6 to 11. |  | some |
| 127 | St. Victor o'Marseilles | 65 | 30-80 | 40% | Heals all lost Endurance and 20% of lost Strength. For a day: Charisma +8 to 15, the skills of the weapons in hand +15 to 24, Speak Common +10 to 19. |  | none |
| 128 | St. Vitus | 48 | 40-99 | 65% | For a day: Charisma +10 to 19, Agility +12 to 23. At the party's next step on the map it is carried to the nearest town. | No Endurance lost. | many |
| 129 | St. Wenceslaus | 44 | 35-99 | 60% | May raise the reputation by 15 to 24 in St. Joachimsthal, Bürglitz, Prag, Kuttenberg, Olmütz or Brünn (nowhere else). For a day: Perception and Strength +6 to 11, Intelligence +8 to 13, the skills of the weapons in hand +8 to 15, Riding +8 to 15. | No reputation with nobles in its help. | some |
| 130 | St. Wilfrid | 25 | 15-65 | 15% | Heals 20% of lost Endurance. For a day: Charisma +6 to 11, Artifice +6 to 11, Speak Common +20 to 39. For a week no ambush takes the party by surprise (bandits and the like). | Not in the game: faster travel. Instead, a week safe from ambushes. | a few |
| 131 | St. Willehad | 35 | 20-99 | 35% | For a day: Agility +8 to 15, Woodwise +8 to 15, Riding +10 to 19. For a week no ambush takes the party by surprise (bandits and the like). |  | many |
| 132 | St. Willebald | 50 | 25-99 | 55% | For a day: Charisma +10 to 19, Woodwise +6 to 11, Speak Common +25 to 49. |  | none |
| 133 | St. Williborord | 43 | 15-50 | 55% | For a day: Charisma +10 to 19, Woodwise +8 to 15, Speak Common +20 to 39. | No reputation in its help. | none |
| 134 | St. Wolfgang | 24 | 20-99 | 35% | For a day: Charisma +8 to 15, Intelligence +6 to 11, Woodwise and Riding +6 to 11, Speak Common +20 to 39. | No reputation in its help. | a few |
| 135 | St. Zita | 23 | 10-65 | 15% | Heals 20% of lost Endurance and 5% of lost Strength. For a day: Perception +6 to 11, Streetwise +6 to 11. |  | a few |
