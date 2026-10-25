# Every text Darklands+ shows

Made by `port/tools/copy_catalog.py` from the source: the words Darklands+ itself writes (the game's own texts are
in its files). One row a text, with where it is. Words in **Terms** below are used the same way everywhere.

## Terms

- **equipped**: worn or held (weapons, armor, shield, a missile weapon); never "in use".
- **the party's pack** (Darklands+, *One shared pack*): one pack for the whole party; spare items are in it, not on a member.
- **party member** (or member): one of the characters in the party.
- **Endurance, Strength, Divine Favor, Virtue**: capitalized, as the game has them.
- **Full Original, Purist, Darklands+**: the presets; **Custom**: own choices.
- **Darklands+** marks what the original game does not have; the Guide colors it.

## Settings: the switches (mods/options.c)

| Where | Text |
|---|---|
| `mods/options.c:103` | these screens drawn anew by Darklands+, in the original's style: the party panel, the menus, the start screen, the cards, shops, the character sheet, loot, alchemy, the store room, the inn and camp, prayer, character creation, party selection, Party Info, the marching order, Enemy Info, the overland view's messages, and Save and Load. They show more (chances, what an item is worth, where an ingredient is sold); the rules are unchanged. When off: the original screens. |
| `mods/options.c:104` | Wares compared with what is equipped |
| `mods/options.c:104` | in a shop, a green arrow by a weapon, armor or shield that is better than what the member has equipped there now, a red one by a worse one (an empty arrow: nothing equipped there); pointing at a ware highlights the equipped item it would replace. |
| `mods/options.c:105` | New party panel |
| `mods/options.c:105` | each member's Endurance, Strength and Divine Favor as labeled bars with numbers. Clicks work as in the original (open a character sheet; give orders in a battle). For a portrait of your own, put NAME.png (or .jpg) in a folder named portraits in the Darklands+ folder, NAME being the member's name. |
| `mods/options.c:106` | Picture in the party panel |
| `mods/options.c:106` | beside each member |
| `mods/options.c:106` | your portrait, else the battle sprite, else the crest\|the battle sprite, else the crest\|the crest\|no picture |
| `mods/options.c:107` | Tooltip delay |
| `mods/options.c:107` | the notes that appear by the pointer: how long the pointer rests on something before its note shows (once one has shown, the next appears at once). |
| `mods/options.c:107` | at once\|after a moment\|after a second |
| `mods/options.c:108` | the game's menus (Game, Orders, Attack, Party ...) open with a right-click or F10 and stay open until you choose or click elsewhere; the entries and what they do are unchanged. When off: the original menu bar, open only while the right button is held. |
| `mods/options.c:109` | New start screen |
| `mods/options.c:109` | Continue (loads the newest save), Load, New party and Quickstart, over the original game's art. |
| `mods/options.c:110` | New card screens |
| `mods/options.c:110` | card text in the typeface you choose, the options as rows to click with their chances beside them; an option that uses an item or a saint lists who can. The game still decides every outcome. When off: the original cards (with crisp text drawn over them). |
| `mods/options.c:111` | New shop screen |
| `mods/options.c:111` | all the wares and all your items in two lists, with Buy and Sell on each row and the equipped items marked; the bottom of the screen describes whatever is under the pointer. Stock and prices are unchanged. |
| `mods/options.c:112` | New character sheet |
| `mods/options.c:112` | the paper doll and the items with Equip, Unequip, Drink and Drop, all the skills on one page, the party's saints and formulae. Drag an item onto a member in the party panel to give it (hold Shift to give one from a stack). Equipping, giving and drinking work as in the original. |
| `mods/options.c:113` | One shared pack for the party |
| `mods/options.c:113` | in the original, each member carries their own items; with this (Darklands+ only), the party shares one pack. Each member keeps only their equipped items and their own potions; everything else goes into the party's pack. Equip and Drink take from the pack, and a shop buys from it. As in the original, only equipped items count toward the load. When the game needs a particular member to have an item (lock picks at a lock, a mount, arrows before a battle), Darklands+ first moves it from the pack to that member. Keep marks an item so it is never sold or dropped. |
| `mods/options.c:114` | New loot screen |
| `mods/options.c:114` | the whole pile and each member's items in two lists; a whole stack moves in one click. The original keys G and P still move every item. |
| `mods/options.c:115` | Shop value in the loot screen |
| `mods/options.c:115` | under the lists, about what a shop would pay for the item under the pointer. |
| `mods/options.c:116` | New alchemy screen |
| `mods/options.c:116` | every known formula in one list, with its ingredients at hand, batches set with - and +, and the chance of success. The chance and the brewing are unchanged. |
| `mods/options.c:117` | Where to buy missing ingredients |
| `mods/options.c:117` | beside an ingredient that runs short, the nearest town seen selling it. |
| `mods/options.c:118` | New store room screen |
| `mods/options.c:118` | everything stored and everything the member carries in two lists, with Take and Put on each row (one or the whole stack), and Put away to store everything not equipped. When off: the original screen, four items at a time. |
| `mods/options.c:119` | One store room shared by every inn |
| `mods/options.c:119` | what you leave with one innkeeper is waiting at every other inn, as with the item boxes of a certain mansion; best not to ask how (100 kinds of items at most). Items already stored in several towns are put together the first time a store room is opened. |
| `mods/options.c:120` | At the inn, rested members earn money |
| `mods/options.c:120` | a member at full Endurance starts the day set to Earn a little money instead of Just Relax (which only refills Endurance). Change it as you like on the inn's screen. |
| `mods/options.c:121` | New inn and camp screen |
| `mods/options.c:121` | each member's activity for the day, its cost, the teachers and their fees, and "Until rested". |
| `mods/options.c:122` | New prayer screen |
| `mods/options.c:122` | the saint's statue, what the saint does, the Divine Favor to spend and the chance (by the original game's rules). |
| `mods/options.c:123` | New character creation |
| `mods/options.c:123` | each step on one page, with what a childhood or profession would give: the attributes afterwards (and the aging past thirty), each skill's gain and how far it can still rise, the Virtue and the saints gained. The rules are unchanged. |
| `mods/options.c:124` | Step-by-step guide |
| `mods/options.c:124` | a short explanation of each step, in the empty column on the left. |
| `mods/options.c:125` | New party selection screen |
| `mods/options.c:125` | the four party slots with their sprites, the saved characters and the selected character's attributes and skills; click to select, double-click to add or remove. |
| `mods/options.c:126` | New Party Info screen |
| `mods/options.c:126` | fame and its title, the date, the place, the wealth, and the party's reputation in every town at once (the original shows one town at a time, under the pointer). |
| `mods/options.c:127` | New marching order screen |
| `mods/options.c:127` | a list to move members up and down, then Done, instead of clicking each member in turn. |
| `mods/options.c:128` | New Enemy Info screen |
| `mods/options.c:128` | E in a battle: every kind of enemy on the field, its armor and weapons, and how well your weapons work against it. |
| `mods/options.c:129` | New overland view messages |
| `mods/options.c:129` | the overland view's messages (a robber knight's land and the like) drawn in the style of the cards. |
| `mods/options.c:130` | New Save and Load screens |
| `mods/options.c:130` | each save listed with its place, date and party; Save asks for its comment beside the list, and Load lists every save (the original lists only nine). |
| `mods/options.c:132` | Chances and hints on the cards |
| `mods/options.c:132` | on a card, the chance of each option that can fail and who would try it; which saints and items it accepts and who has them; a lectern by an option that brings a teacher (skill, level, fee); the options at once without the fade-in, and 1 to 9 to choose them; at the docks, a small map of where each boat goes. Tooltips say what an item, saint or formula is good for. |
| `mods/options.c:133` | Chances of success |
| `mods/options.c:133` | beside each card option that can fail, with the member who would try it (the original shows one at a time while Shift is held). |
| `mods/options.c:133` | Always, as numbers\|As the difficulty level allows\|Off |
| `mods/options.c:134` | The best member speaks for the party |
| `mods/options.c:134` | when a card tests the leader, the member best at it takes the test instead, and the one with the best Charisma haggles in shops; the chance shown is theirs, with their head. The arrow at the end of the chance shows every member's: a click on one has that member speak instead. After the card, your chosen leader is leader again. (When off, the leader speaks, and the arrow still lets you pick another.) |
| `mods/options.c:135` | Saints and items on cards |
| `mods/options.c:135` | which saints and items a card accepts, and who knows or carries them (as the Darklands Companion, a players' reference, did: saints are listed even when nobody knows them yet). |
| `mods/options.c:136` | what the item, saint or formula under the pointer is good for (in shops, the loot screen, character sheets, and the prayer and alchemy screens). |
| `mods/options.c:137` | Cards without the fade-in |
| `mods/options.c:137` | a card shows its options at once, without the slow fade-in. |
| `mods/options.c:138` | Number keys for card options |
| `mods/options.c:138` | 1 to 9 choose a card's options. |
| `mods/options.c:139` | Who has work |
| `mods/options.c:139` | in the Go to list, beside the Fugger, the Medici, the Hanse, the cathedral, the council, the lord and the market stalls: the chance that asking brings a quest, the date they will have one again, or a note that their quest is under way or its reward is waiting. |
| `mods/options.c:140` | a lectern beside each card option that brings a teacher to the inn, with the skill, the teacher's level and the fee; the margin and the world map (M) show which teachers each town can offer. |
| `mods/options.c:141` | G on a town's card lists its places; pick one and the party goes there, clicking through the menus for you. |
| `mods/options.c:142` | Map at the docks |
| `mods/options.c:142` | beside the docks' list of boats, a small map of the towns they go to and the way each takes along the rivers and coasts; a click on a town takes that boat. |
| `mods/options.c:143` | Note each town's shops and wares |
| `mods/options.c:143` | each town's shops, their wares, qualities and prices, noted on arrival (shown on the world map, M). Switched in F12 > Journal. |
| `mods/options.c:144` | every text the cards show, word for word, with the date, the place and the choice made, so a clue read weeks ago can be found again (F12 > Journal > Chronicle, where it is switched). |
| `mods/options.c:146` | Thrown weapons picked up after a battle |
| `mods/options.c:146` | when the loot screen opens after a battle, the javelins, throwing axes, throwing knives and darts each member threw are taken from the pile and given back to them. |
| `mods/options.c:147` | Endless ammunition and thrown weapons |
| `mods/options.c:147` | arrows, quarrels and balls, and javelins, throwing axes, throwing knives and darts, never run out (one is enough to shoot or throw); enemies still use up theirs. When off: each shot uses one, and about 7 in 10 arrows and quarrels can be picked up in the loot afterwards. |
| `mods/options.c:148` | Endless ammunition |
| `mods/options.c:148` | arrows, quarrels and balls never run out (one is enough to shoot); enemies still use up theirs. When off: each shot uses one, and about 7 in 10 arrows and quarrels can be picked up in the loot afterwards. |
| `mods/options.c:149` | Endless thrown weapons |
| `mods/options.c:149` | javelins, throwing axes, throwing knives and darts never run out either (one is enough to throw). |
| `mods/options.c:150` | Aging during play |
| `mods/options.c:150` | characters go on aging during play (Darklands+ only; the original ages them only during character creation): from thirty on, a little each month, more with age (by seventy, a total loss of 25 Endurance, 21 Strength and Agility each, 10 Perception and Charisma each, and 5 Intelligence). |
| `mods/options.c:151` | Warn about permanent losses |
| `mods/options.c:151` | anything lost for good (an attribute's maximum, a skill, armor eaten by acid or fire, the toll of the years) pauses the game with a window saying exactly what was lost, and by whom. |
| `mods/options.c:153` | Easier battle controls |
| `mods/options.c:153` | click a member (or their box in the party panel), then an enemy to attack it or the ground to walk there: one click gives the order, and while paused the next member without orders is chosen. A bow or sword on each shooter's box says whether a click shoots or fights. Order buttons with icons, right-drag to move the view, selecting a member doesn't move the view, a shorter wait at the end of a battle, and Z to fast-forward (Z again: back to the chosen speed). |
| `mods/options.c:154` | Battle info on screen |
| `mods/options.c:154` | a bar over each wounded enemy, and pointing at someone shows who it is, how badly hurt, their melee rating, aim and weapons; the line of a shot with what is in the way; what a click will do; an arrow to enemies off screen; skill gains rising over the fighter; a reload bar under a shooter; and the battle log (every blow, parry and fall). |
| `mods/options.c:155` | Pauses in battle |
| `mods/options.c:155` | the battle pauses when a member falls and when enemies waiting in a castle, dungeon or mine come out to fight. |
| `mods/options.c:156` | Smooth, steady scrolling |
| `mods/options.c:156` | the battle view slides instead of jumping, scrolls at a steady pace, and resting the pointer on the edge scrolls it without Shift. |
| `mods/options.c:157` | Pause when a member falls |
| `mods/options.c:158` | Pause when badly hurt |
| `mods/options.c:158` | the battle pauses when a party member's Endurance drops below a quarter. |
| `mods/options.c:159` | Pause when enemies come out |
| `mods/options.c:159` | the battle pauses when enemies waiting in a castle, dungeon or mine come out to fight. |
| `mods/options.c:160` | Improved combat |
| `mods/options.c:160` | the party fights on sensibly between orders. A member with a bow, crossbow, gun or thrown weapon shoots first and keeps shooting: when the target falls, at the nearest enemy in sight; with no clear line (a wall, a tree or someone standing between, as the aiming line shows), the member moves to the nearest spot a few paces off that has one (or walks up until there is one); hand to hand only once an enemy is close or there is nothing left to shoot. A member whose enemy falls turns on one close by, Parry, Berserk and Vulnerable stay as ordered, and a click on an enemy opens with a Vulnerable attack (aimed at the gaps in its armor) when only that would get through its armor. When off: the original's ways. |
| `mods/options.c:161` | Click an enemy to attack it |
| `mods/options.c:161` | a click on an enemy sends the selected member to attack it (Std Attack), even before an attack is chosen. |
| `mods/options.c:162` | Bow or sword on each member's box |
| `mods/options.c:162` | in a battle, a member holding a missile weapon shows a bow or a sword on their box in the party panel: whether a click on an enemy makes them shoot or fight hand to hand. Click it to switch; each member keeps their choice. |
| `mods/options.c:163` | Click a member's box to give them orders |
| `mods/options.c:163` | in a battle, a click on a member's box in the party panel selects them for orders, as 1 to 5 do (a double-click also moves the view to them); a right-click on the box opens their character sheet. When off: a click on the box opens the sheet, as in the original. |
| `mods/options.c:164` | One click gives an order |
| `mods/options.c:164` | with a member selected, a click on an enemy or the ground gives the order at once, as do orders that need no target (Halt, a door, the stairs). An attack picked from the Orders menu or the buttons (Std Attack, Vulnerable, Berserk, Parry) goes against the enemy the member is already fighting. If selecting the member paused the battle, it resumes with the order. When off: a click only marks the target, and Enter, Space or selecting another member gives the order, as in the original. |
| `mods/options.c:165` | Then select the next member without orders |
| `mods/options.c:165` | while the battle is paused, giving an order selects the next member on screen who has no order, so at the start of a battle each order is a single click on an enemy. Space resumes the battle. |
| `mods/options.c:166` | Order buttons over the field |
| `mods/options.c:166` | when the window has no room for a right margin (narrower than 16:9), the order buttons appear in a strip at the bottom of the battlefield while a member is selected, so no right-click menu is needed. |
| `mods/options.c:167` | Icons on the order buttons |
| `mods/options.c:167` | each order button in the right margin (or over the field) shows an icon beside its name: a sword for Std Attack, a shield for Parry, footprints for Walk towards, a white flag for Surrender. |
| `mods/options.c:168` | Show the line of a shot |
| `mods/options.c:168` | when a member is about to shoot or throw at the enemy under the pointer, a line runs from them to it. Anything in the way (a wall, a tree, someone standing between) is outlined in red, and the line turns red past it: the shot can't be taken from there. |
| `mods/options.c:169` | Say what a click will do |
| `mods/options.c:169` | with a member selected, a short label by the pointer says what a click there does when it isn't a plain attack or walk (shoot, throw, flee, a Vulnerable or Berserk attack) or why it can't (no way there, no clear shot). |
| `mods/options.c:170` | Drag to move the view |
| `mods/options.c:170` | hold the right (or middle) button and move the mouse to drag the battle view; a right-click without moving still opens the menus. |
| `mods/options.c:171` | Selecting a member doesn't move the view |
| `mods/options.c:171` | 1 to 5 (or a click on a member's box) select a member without moving the view; press twice to move the view to the member and follow them. Selecting a member on another floor still moves the view to them. |
| `mods/options.c:172` | Smooth scrolling |
| `mods/options.c:172` | the battle view slides to each new place instead of jumping. |
| `mods/options.c:173` | Steady scrolling |
| `mods/options.c:173` | the battlefield scrolls at a steady pace (holding Shift at the edge no longer scrolls all the way to the end of the field at speed). |
| `mods/options.c:174` | Edge scrolling without Shift |
| `mods/options.c:174` | resting the pointer on the edge of the screen scrolls the battlefield; no Shift needed. |
| `mods/options.c:175` | Info under the pointer |
| `mods/options.c:175` | a bar over each wounded enemy with the Endurance it has left (Alt: over all); pointing at someone shows who it is, how badly hurt they are, their melee rating (attack and defense hand to hand), their aim with a missile weapon, and their weapons. |
| `mods/options.c:176` | Enemy Endurance as numbers |
| `mods/options.c:177` | Arrow to enemies off screen |
| `mods/options.c:177` | when no enemy is on screen, an arrow at the edge points to the nearest one; click it to move the view there. |
| `mods/options.c:178` | Show skill gains |
| `mods/options.c:178` | a skill raised in a fight rises in gold over the fighter's head (+2 Edged), and a note of it shows in the member's box in the party panel. |
| `mods/options.c:179` | a thin amber line under a member readying a shot (bow, crossbow, gun or thrown weapon), filling up until they shoot. |
| `mods/options.c:180` | every blow, parry, wound and fall, in the left margin on a wide window, otherwise over the field (F12 > Journal > Battle log keeps it). |
| `mods/options.c:181` | Level map (Tab) |
| `mods/options.c:181` | Tab in battles, castles, mines and dungeons: a map of the level as far as the party has seen it; a click on it walks the party there. |
| `mods/options.c:182` | The whole level on the level map |
| `mods/options.c:182` | Tab shows the whole level, not only what the party has seen. |
| `mods/options.c:183` | More day jobs |
| `mods/options.c:183` | nine jobs the original never offers (mercenary, guard sergeant, bodyguard, courtier, scribe, clerk, troubadour, entertainer, preaching friar) turn up in towns with the right places: soldiers' jobs by an arsenal or fortress, a lord's household at a fortress, clerks at a town hall, the great houses in a big city, friars by a monastery. Each member still takes the best-paid job they qualify for. |
| `mods/options.c:184` | Enemies still waiting |
| `mods/options.c:184` | under the enemy list, how many enemies the party has yet to meet in a castle, dungeon or mine, floor by floor, and which floor its master is on (knowledge the party could not have). |
| `mods/options.c:185` | Shorter wait at the end of a battle |
| `mods/options.c:185` | when the last enemy falls, the field stands still for one second instead of three and a half before the loot, and "Victory! The battle is won." (or the like) stays for three seconds instead of eight. The original's long waits looked like a freeze. |
| `mods/options.c:186` | Fighters find their way around obstacles |
| `mods/options.c:186` | fighters plan their way around walls, furniture and each other; in the original they often give up on long or winding routes (members get stuck in mines). Enemies get it too, so battles are harder. In a crowd, a blocked fighter finds another way at once instead of standing still for seconds, and enemies at the back wait or pick another target rather than take a long detour. |
| `mods/options.c:187` | Battle speed |
| `mods/options.c:187` | walking (in dungeons, castles and mines too), attacks, drinking potions and calling on saints all go faster; pausing (Space) works as before. |
| `mods/options.c:187` | as the original\|1.5x\|2x\|3x |
| `mods/options.c:188` | Fast forward (Z) |
| `mods/options.c:188` | Z in a battle runs it six times faster; Z again goes back to the chosen speed. |
| `mods/options.c:190` | World map (M) |
| `mods/options.c:190` | M opens a map of the whole land (the towns, the quests, where saints and formulae are found). |
| `mods/options.c:191` | the party follows the roads to a place you pick: double-click it (or Travel there) on the world map (M), or Ctrl+click it in the overland view. Any key or click stops it. |
| `mods/options.c:192` | Auto-travel speed |
| `mods/options.c:192` | walking pace\|4x\|instant |
| `mods/options.c:193` | Names and quest markers in the overland view |
| `mods/options.c:193` | the towns and places named in the overland view, each quest's icon above its place, and a sign beside a town that bars the party or where the watch is after it. |
| `mods/options.c:194` | Place names in the overland view |
| `mods/options.c:194` | the towns and places named in the overland view. |
| `mods/options.c:195` | Quest markers in the overland view |
| `mods/options.c:195` | each quest's icon above its place in the overland view, and a sign beside a town that bars the party or where the watch is after it. |
| `mods/options.c:196` | What unvisited places offer |
| `mods/options.c:196` | the saints a library teaches, the formulae an alchemist sells and a shop's goods, also in towns and monasteries the party has not visited: on the world map, on the character sheet and beside an ingredient that runs short. When off: only what the party has seen. (How good each town's smiths and shops are is shown either way.) |
| `mods/options.c:197` | Encounters on the road |
| `mods/options.c:197` | how often travel brings an encounter. |
| `mods/options.c:197` | a quarter\|half\|as the original\|double |
| `mods/options.c:198` | Night encounters in the streets |
| `mods/options.c:198` | how often thieves and the watch turn up in town at night (in the original, cutthroats about 8 times in 10 in the side streets for a party with little Stealth and Streetwise, and the watch now and then on the main street). |
| `mods/options.c:198` | a quarter\|half\|as the original\|double |
| `mods/options.c:203` | the typeface of the cards and the new screens (the game's own lettering drawn crisp, or one of several period hands). |
| `mods/options.c:204` | The same typeface for the margins |
| `mods/options.c:204` | the side panels, tooltips and the place names on the world map and in the overland view in the chosen typeface too. |
| `mods/options.c:205` | Spell out abbreviations |
| `mods/options.c:205` | the character sheet's abbreviations written out (wPol: Polearm Wpns). |
| `mods/options.c:206` | Money as coins |
| `mods/options.c:206` | florins, groschen and pfennigs as small coins (1 12 4) instead of 1fl 12gr 4pf. |
| `mods/options.c:207` | Letter edges |
| `mods/options.c:207` | plain, a shadow or an outline around the letters drawn over pictures (0 plain, 1 shadow, 2 outline). |
| `mods/options.c:208` | Darken faint text |
| `mods/options.c:208` | faint text darkened (or lightened) against its background so it stays readable. |
| `mods/options.c:209` | the size of the text in the margins, the tooltips and the place names (world map and overland view), in percent. |
| `mods/options.c:210` | Icons beside the skills |
| `mods/options.c:210` | small icons beside the skills and attributes. |
| `mods/options.c:211` | Skill icons' look |
| `mods/options.c:211` | on the character sheet: heraldic charges (2) or in color (0); in color elsewhere. |
| `mods/options.c:212` | A potion's grade |
| `mods/options.c:212` | how a potion's grade shows on its flask (4 chevrons on a badge, 3 chevrons, 1 dots on a strip, 0 small dots). |
| `mods/options.c:213` | The Philosopher's Stone |
| `mods/options.c:213` | how the stone is pictured (2 the squared circle, 1 a rough stone, 0 an egg). |
| `mods/options.c:214` | the clock shown while hours pass (styles by number). |
| `mods/options.c:215` | the plaited band and the gilt corners around the windows. |
| `mods/options.c:216` | the band around the windows (styles by number). |
| `mods/options.c:217` | Button style |
| `mods/options.c:217` | the look of the buttons (styles by number). |
| `mods/options.c:218` | Replacement art: show file names |
| `mods/options.c:218` | the file each picture would be replaced from, for those making replacement art. |
| `mods/options.c:219` | Soft scaling |
| `mods/options.c:219` | the game's picture blurred a little, as an old monitor would, rather than in sharp pixels (Soft in F12 > Settings > Screen). |
| `mods/options.c:220` | Rounded edges |
| `mods/options.c:220` | the stair-steps of diagonal edges rounded off (Rounded edges in F12 > Settings > Screen): 0 as drawn, 1 and 2 rounded. |
| `mods/options.c:221` | System pointer |
| `mods/options.c:221` | the system's mouse pointer over the game too. |
| `mods/options.c:222` | Smooth pointer |
| `mods/options.c:222` | the game's mouse pointer moves as smoothly as the system's. |
| `mods/options.c:223` | Size of the menus |
| `mods/options.c:223` | the size of Darklands+'s menus in percent (0: the system's scale), after a restart. |
| `mods/options.c:224` | Pause in the background |
| `mods/options.c:224` | the game pauses while its window is in the background. |
| `mods/options.c:225` | Intro at start-up |
| `mods/options.c:225` | the intro: 0 nothing, 1 the last scene, 2 the whole (-1: the whole the first time). |
| `mods/options.c:226` | Drawing cache |
| `mods/options.c:226` | the ornate frames drawn once and kept, not drawn again each time: 1 automatic, 0 off; 2 and 3 are two other ways of keeping them, to try if frames look wrong on a computer. |
| `mods/options.c:227` | Encounters from mods |
| `mods/options.c:227` | the new encounters that mods add (F12 > Mods) come up on the road. |
| `mods/options.c:228` | A seal on encounters from mods |
| `mods/options.c:228` | a small red seal in the corner of a card that comes from a mod, not the original game. |
| `mods/options.c:229` | How often encounters from mods come up |
| `mods/options.c:229` | in percent: at 100, each is as likely as any one of the original encounters for that terrain. |
| `mods/options.c:230` | Button prompts |
| `mods/options.c:230` | the controller's buttons on screen: 0 the device in use, 1 keys and mouse, 2 Xbox, 3 PlayStation, 4 Nintendo. |
| `mods/options.c:231` | the next new game keeps one save, saved over by the game itself as it goes (every card, each arrival, before each battle, on quitting); no loading back while it runs; when the party's journey ends, its save is deleted. |
| `mods/options.c:232` | Back up saves |
| `mods/options.c:232` | a copy of each save before it is overwritten. |
| `mods/options.c:233` | Autosaves kept |
| `mods/options.c:233` | how many autosaves are kept. |
| `mods/options.c:234` | Quick saves kept |
| `mods/options.c:234` | how many quick saves are kept. |
| `mods/options.c:235` | Music volume |
| `mods/options.c:236` | Effects volume |
| `mods/options.c:237` | Music in the menus |
| `mods/options.c:237` | the music goes on while Darklands+'s menus are open. |
| `mods/options.c:238` | MT-32 display |
| `mods/options.c:238` | the MT-32's little display shown at the top right when the game puts a message on it. |
| `mods/options.c:239` | Quests sorted by |
| `mods/options.c:239` | the margin's quests: 0 by deadline, 1 nearest first, 2 as taken. |
| `mods/options.c:241` | Replacement art |
| `mods/options.c:242` | Battle sounds from a sound pack |
| `mods/options.c:243` | Side margins |
| `mods/options.c:244` | Names in full |
| `mods/options.c:245` | Where armor is worn |
| `mods/options.c:246` | Quick saves and autosaves |
| `mods/options.c:247` | Autosave on arriving |
| `mods/options.c:248` | Autosave before a battle |
| `mods/options.c:249` | Back to the start screen when the journey ends |
| `mods/options.c:251` | "Fehde!" ambush |
| `mods/options.c:251` | a road ambush by soldiers of a private feud, written as a variant of the mercenaries' ambush and never used: half the time that ambush comes up, the Fehde comes instead, with its own texts but the same odds and battle (Soldiers led by a Knight). |
| `mods/options.c:252` | Town hospice |
| `mods/options.c:252` | a town's hospice by day, written but never made playable: give alms, work a day beside the nuns (Healing), have the party's wounds tended for a fee, or beg to be tended for nothing. The church district leads there, and Go to (G) lists it. |
| `mods/options.c:253` | Cities at war |
| `mods/options.c:253` | coded and never switched on. Now and then a city prepares for war for a few weeks, and some then stand a siege: prices rise, the news tells of it, Soldiers' Road and its market open, the docks are guarded or barricaded, and the gate turns travelers away (talk, bluff, sneak, a farmer's hay cart, or the walls, where patrols take you for spies). A save made during a war carries the war on if loaded in the original game. |
| `mods/options.c:254` | City politics |
| `mods/options.c:254` | the campaign written and never switched on. Now and then a city has a season of unrest: rumors in the news and the guild quarters lead to the rebels' leader or the government's man, the party takes a side, posts handbills, sways the guilds, and a war council sets the day of a march on the town hall or of the arrests. |
| `mods/options.c:255` | Black Cloud in battle |
| `mods/options.c:255` | the party can throw Black Cloud in a battle too. The manual left it out ("the screen takes too long" on the machines of 1992), but the battle rules handle its smoke: shots through it hit a fifth as often, and fighting from inside it gives -15 to hit. Enemies and cards already use it. |
| `mods/options.c:256` | Honor Roll of Glory (hall of fame) |
| `mods/options.c:256` | the hall of fame left unfinished in the game (its picture and table were in the files, never used): a party enters it when its journey ends, and F12 > Journal > Honor Roll shows it. |
| `mods/options.c:277` | Takes effect when the next new game starts. |
| `mods/options.c:282` | Fixes for bugs of the original |
| `mods/options.c:329` | Full Original |
| `mods/options.c:329` | Darklands as it was in 1993 (version 483.07), bugs and all: everything Darklands+ adds is off, and the original game's sound setup is used. |
| `mods/options.c:330` | Saves load in the original game and back. |
| `mods/options.c:331` | The original game with its bugs fixed: its own screens, rules and graphics. |
| `mods/options.c:332` | Saves load in the original game and back. |
| `mods/options.c:333` | The original rules with the original's bugs fixed, the new screens, on screen what the original keeps behind keys or only in its manual, and less busywork: auto-travel, Go to, easier battle controls, one shared pack, one store room for every inn, endless ammunition. Easier play is offered separately, as built-in mods (F12 > Mods). |
| `mods/options.c:334` | Saves load in the original game and back, unless a member uses an enemy sprite. |
| `mods/options.c:456` | # Darklands+ switches  Every switch: its key (its name for good: never renamed, never reused), what it does, why it exists, the group switch that sets it (if any), its class (what kind of change: fix, show, ease, plus, easy, experiment, taste, tool) and its value in the original game / Darklands+ / for easy play (the column of the former Easy preset). Made by the game itself (`DL_DUMP_SWITCHES=docs/SWITCHES.md`, from `port/mods/options.c` and `port/mods/optwhy.c`); `tools/check_switches.py` fails if a key disappears without being listed as retired.   |
| `mods/options.c:481` |  ## Retired keys  No longer offered; not to be used for anything else.   |

## Settings: why each switch (mods/optwhy.c)

| Where | Text |
|---|---|
| `mods/optwhy.c:12` | The original screens are hard to read when scaled up and hide much behind keys; one switch, since nobody wants half the screens new and half old. |
| `mods/optwhy.c:13` | The original needs several clicks and menus per order; one click per order makes real-time battles playable. One switch: the parts depend on one another. |
| `mods/optwhy.c:14` | The original shows almost nothing of a battle's state on the field; these show what the game already knows, nothing more. |
| `mods/optwhy.c:15` | Things that need the player's attention happen while the battle runs on. |
| `mods/optwhy.c:16` | The original's view jumps and races, which loses the player's bearings. |
| `mods/optwhy.c:17` | The original hides the odds and what a card accepts behind Shift and the manual; the game's own numbers, shown. |
| `mods/optwhy.c:19` | Comparing a ware with what is equipped meant opening the sheet. |
| `mods/optwhy.c:20` | The original panel shows bars without numbers. |
| `mods/optwhy.c:21` | A matter of taste: portrait, sprite or crest. |
| `mods/optwhy.c:22` | A matter of taste. |
| `mods/optwhy.c:23` | The original menu bar is open only while the right button is held. |
| `mods/optwhy.c:24` | Loading the newest save was several screens away. |
| `mods/optwhy.c:25` | The cards' text is the heart of the game and was the hardest to read. |
| `mods/optwhy.c:26` | The original shows a few items at a time and no comparison. |
| `mods/optwhy.c:27` | The original splits the sheet over several screens of small text. |
| `mods/optwhy.c:28` | The original moves one item per click. |
| `mods/optwhy.c:29` | Deciding what to carry needs what it is worth. |
| `mods/optwhy.c:30` | The original brews one formula at a time and hides the chance. |
| `mods/optwhy.c:31` | Missing ingredients meant searching the towns by memory. |
| `mods/optwhy.c:32` | The original shows four stored items at a time. |
| `mods/optwhy.c:33` | The original hides the cost and the teachers' terms. |
| `mods/optwhy.c:34` | The original hides what a saint does and the chance. |
| `mods/optwhy.c:35` | The original hides what each choice will give. |
| `mods/optwhy.c:36` | Character creation is the first wall for new players. |
| `mods/optwhy.c:37` | The original picks party members one click at a time without details. |
| `mods/optwhy.c:38` | The original shows one town's reputation at a time. |
| `mods/optwhy.c:39` | The original sets the order by clicking each member in turn. |
| `mods/optwhy.c:40` | The original Enemy Info is one enemy at a time, without armor or weapons. |
| `mods/optwhy.c:41` | The overland view's messages were in the old style among the new screens. |
| `mods/optwhy.c:42` | The original lists only nine saves, without their place or party. |
| `mods/optwhy.c:44` | Moving spare items from member to member is busywork; with one pack for the party, each member's equipped items still count toward their load. |
| `mods/optwhy.c:45` | A store room per town made stored items as good as lost. |
| `mods/optwhy.c:46` | A rested member relaxing gains nothing; earning was the better choice the player always had to make by hand. |
| `mods/optwhy.c:47` | The original tests only the leader, so the player swaps leaders before each card by hand. |
| `mods/optwhy.c:48` | The original shows a chance only while Shift is held, one option at a time. |
| `mods/optwhy.c:49` | Which saints and items a card accepts was known only from the Darklands Companion. |
| `mods/optwhy.c:50` | What an item, saint or formula is good for was in the manual only. |
| `mods/optwhy.c:51` | The fade-in is a wait each card. |
| `mods/optwhy.c:52` | Choosing with the keyboard is quicker than clicking. |
| `mods/optwhy.c:53` | Finding which office has work meant visiting each. |
| `mods/optwhy.c:54` | Teachers are easy to miss: nothing in the original says an option brings one. |
| `mods/optwhy.c:55` | Reaching one place in a town took many clicks through its menus, none of them a real choice. |
| `mods/optwhy.c:56` | Boat destinations are names without a map. |
| `mods/optwhy.c:57` | The player otherwise keeps notes on paper. |
| `mods/optwhy.c:58` | A record of the journey the original does not keep. |
| `mods/optwhy.c:59` | Gathering thrown weapons after each battle is busywork. |
| `mods/optwhy.c:60` | Counting arrows and gathering javelins is bookkeeping, not play; enemies still run out. |
| `mods/optwhy.c:61` | The original overland view names nothing and shows no quest. |
| `mods/optwhy.c:62` | Counting arrows is bookkeeping, not play; enemies still run out. |
| `mods/optwhy.c:63` | The same for thrown weapons. |
| `mods/optwhy.c:64` | Creation ages characters but play does not, so starting with older characters cost nothing later; this keeps age a cost. |
| `mods/optwhy.c:65` | Permanent losses happen silently in the original. |
| `mods/optwhy.c:66` | Jobs written into the game's tables and never offered. |
| `mods/optwhy.c:68` | (part of Pauses in battle) |
| `mods/optwhy.c:69` | (part of Pauses in battle) |
| `mods/optwhy.c:70` | (part of Pauses in battle) |
| `mods/optwhy.c:71` | Between orders the original's members stand idle or fight poorly (shooters walk into melee, freeze without a line of fire); this is what a player would order each time. |
| `mods/optwhy.c:72` | (part of Easier battle controls) |
| `mods/optwhy.c:73` | (part of Easier battle controls) |
| `mods/optwhy.c:74` | (part of Easier battle controls) |
| `mods/optwhy.c:75` | (part of Easier battle controls) |
| `mods/optwhy.c:76` | (part of Easier battle controls) |
| `mods/optwhy.c:77` | (part of Easier battle controls) |
| `mods/optwhy.c:78` | (part of Easier battle controls) |
| `mods/optwhy.c:79` | (part of Easier battle controls) |
| `mods/optwhy.c:80` | (part of Easier battle controls) |
| `mods/optwhy.c:81` | (part of Easier battle controls) The original's long wait at the end looked like a freeze. |
| `mods/optwhy.c:82` | (part of Smooth, steady scrolling) |
| `mods/optwhy.c:83` | (part of Smooth, steady scrolling) |
| `mods/optwhy.c:84` | (part of Smooth, steady scrolling) |
| `mods/optwhy.c:85` | (part of Battle info on screen) |
| `mods/optwhy.c:86` | (part of Battle info on screen) |
| `mods/optwhy.c:87` | (part of Battle info on screen) |
| `mods/optwhy.c:88` | (part of Battle info on screen) |
| `mods/optwhy.c:89` | (part of Battle info on screen) |
| `mods/optwhy.c:90` | (part of Battle info on screen) |
| `mods/optwhy.c:91` | (part of Battle info on screen) |
| `mods/optwhy.c:92` | Easier: exact numbers the party could not know. |
| `mods/optwhy.c:93` | Easier: knowledge the party could not have. |
| `mods/optwhy.c:94` | Dungeons and castles are hard to find one's way in without a map. |
| `mods/optwhy.c:95` | Easier: the parts not yet seen. |
| `mods/optwhy.c:96` | In the original, fighters get stuck in mines and long rooms. |
| `mods/optwhy.c:97` | A matter of taste: some find the original's battles slow. |
| `mods/optwhy.c:98` | (part of Easier battle controls) Battles already won take long to finish. |
| `mods/optwhy.c:100` | Steering across the overland view step by step took many key presses, none of them a real choice. |
| `mods/optwhy.c:101` | A matter of taste. |
| `mods/optwhy.c:102` | The original overland view names nothing. |
| `mods/optwhy.c:103` | Where a quest leads was in the player's notes only. |
| `mods/optwhy.c:104` | Easier: knowledge the party could not have. |
| `mods/optwhy.c:105` | The original has no map of the whole land. |
| `mods/optwhy.c:106` | Easier: fewer interruptions. |
| `mods/optwhy.c:107` | Easier: the original's night streets are near-certain ambushes. |
| `mods/optwhy.c:109` | The original's small lettering is hard to read on a large screen. |
| `mods/optwhy.c:110` | Replacement pictures, for those who make them. |
| `mods/optwhy.c:111` | A matter of taste. |
| `mods/optwhy.c:112` | Wide screens have room the original left black. |
| `mods/optwhy.c:113` | The original cut names short to fit its screens. |
| `mods/optwhy.c:114` | V:/L: prefixes are cryptic. |
| `mods/optwhy.c:115` | Saving took several menus. |
| `mods/optwhy.c:116` | Players asked for a game where every choice stands. |
| `mods/optwhy.c:117` | A crash or a bad fight cost hours. |
| `mods/optwhy.c:118` | A lost battle cost what came before it. |
| `mods/optwhy.c:119` | The original quits to DOS at the end. |
| `mods/optwhy.c:121` | Written by MicroProse and never used. |
| `mods/optwhy.c:122` | Written by MicroProse and never made playable. |
| `mods/optwhy.c:123` | Coded by MicroProse and never switched on. |
| `mods/optwhy.c:124` | Written by MicroProse and never switched on. |
| `mods/optwhy.c:125` | Left out in 1992 for the machines' speed; the rules handle it. |
| `mods/optwhy.c:126` | Left unfinished in the game's files. |
| `mods/optwhy.c:151` | Puts right a bug of the original (the description says which). |
| `mods/optwhy.c:152` | A matter of taste, or of the computer it runs on; no preset touches it. |

## Fixes (mods/fixes.c)

| Where | Text |
|---|---|
| `mods/fixes.c:230` | Blessings no longer cost attribute points for good |
| `mods/fixes.c:231` | In the original, a second blessing on the same attribute, or one near 99, took points away for good when it wore off. A boost to Endurance or Strength was taken off in full when it ran out, even after a night's rest or a wound had already used it up (a badly wounded member could die of it). A lowering given to the whole party (the Cavern of Fire and Hail) stopped at the first member boosted above it. Now a blessing takes back only what it really added and what is still above the maximum, and a lowering reaches every member and is given back when it ends. |
| `mods/fixes.c:232` | No invulnerability from fire protection |
| `mods/fixes.c:233` | In the original, the fire protection from St. Polycarp, Firewall and the like also counted as armor (+16 to +64) that never wore off. It now protects against fire only. |
| `mods/fixes.c:234` | No "(null)" or empty items |
| `mods/fixes.c:235` | In the original, items named "(null)", which are not real items, and empty stacks could turn up among a member's items, where they could be neither used nor sold. They are no longer added. |
| `mods/fixes.c:236` | Castle battles end when the knight dies |
| `mods/fixes.c:237` | In a castle battle only the knight's death ends the fight, but the original did not save it: after saving and loading during the battle, the knight was dead yet leaving always counted as fleeing. His death now counts after loading too. |
| `mods/fixes.c:238` | Robber-knight bounties from merchants pay out properly |
| `mods/fixes.c:239` | The original looked up the wrong quest giver, which garbled names, corrupted memory and let the reward be claimed again and again. It now finds the right one. |
| `mods/fixes.c:240` | Brewing uses up all the ingredients |
| `mods/fixes.c:241` | The original took only part of them when an ingredient was spread over several stacks; now it takes them all. |
| `mods/fixes.c:242` | No more items lost to a member with no room |
| `mods/fixes.c:243` | A member carries at most 64 kinds of items; in the original, what the game gave to a member who already had 64 simply vanished (quest items too). It now goes to someone with room. Buying when nobody had room took the money for nothing, and taking an item from a store room for a member with no room lost it: both are now refused. |
| `mods/fixes.c:244` | Saving and loading stay possible after the inn |
| `mods/fixes.c:245` | Leaving the inn's lodging screen in some ways (e.g. a click on an empty party slot) left Save and Load disabled until a character sheet was opened. They are now enabled again on leaving. |
| `mods/fixes.c:246` | Members keep their colors (the "neon hair" bug) |
| `mods/fixes.c:247` | In the original, members' colors and sprites sometimes changed to garish ones after an encounter. Their own colors are now kept and put back. |
| `mods/fixes.c:248` | Skip the manual check |
| `mods/fixes.c:249` | The game no longer asks for an alchemical symbol from the printed manual when the party first sets out. |
| `mods/fixes.c:250` | Weapon potions wear off and don't ruin weapons |
| `mods/fixes.c:251` | Deadly Blade, Strongedge and Great Power never wore off, and their bonuses piled up past what the game can hold: after a few potions (or one Great Power), weapons, especially bows, hit for almost nothing. They now wear off after about a day, and several at once add up without going past that limit. |
| `mods/fixes.c:252` | No quest destinations off the edge of the land |
| `mods/fixes.c:253` | One cave lies a row beyond the bottom edge of the land, so quests sent there could never be finished. It is moved onto the land. |
| `mods/fixes.c:254` | Weapon practice trains the leader's best weapon skill |
| `mods/fixes.c:255` | Where a card lets the leader practice with the weapon he or she is best at, the original always trained Missile Devices. Now it trains the leader's best weapon skill. |
| `mods/fixes.c:256` | Robbers take only what is there (no crash afterwards) |
| `mods/fixes.c:257` | When robbers took the party's items, the game removed one item too many; a member left with nothing ended up with -1 items, and if he or she later died in battle, the game crashed. Now only what is there is taken; saved games with the fault are repaired automatically. |
| `mods/fixes.c:258` | Nothing lost in store rooms |
| `mods/fixes.c:259` | A store room holds 100 kinds of items, and its count of any one item ran over past 127 (shown as a negative number, then lost). Putting more in is now refused, with a message saying why. |
| `mods/fixes.c:260` | No crash on the developers' test card |
| `mods/fixes.c:261` | Card 0 is the developers' test card ("Try it...you will like it!"). A card that returns to the card it came from went to card 0 when it had come from no card, and the game stopped on the test card's missing picture (pics\\xhouse3.pic). It now goes on to the overland view, or to the town the party is in. |
| `mods/fixes.c:262` | Only the armor worn wears out |
| `mods/fixes.c:263` | Identical pieces of armor are kept as one stack, so when acid or fire wore down the one being worn, every spare of the same kind and quality carried by that member lost quality too, and all were destroyed with it. Now only the worn one suffers. |
| `mods/fixes.c:264` | Training at the inn finds the town's teachers |
| `mods/fixes.c:265` | When another town also had teachers at the same time, the inn's Train or Study listed nobody and showed a cost like -4096 pfennigs (and a purse that grew); the day then passed with nothing learned. The inn now lists this town's teachers and charges their fee. |
| `mods/fixes.c:266` | Card options no longer grayed out by mistake |
| `mods/fixes.c:267` | Getting out over a city wall, the bribe could be offered without the money (the money check was on the option to crawl out through the sewer instead); at a rebel crowd's fortress the climb with a rope was grayed out when the party had a rope; climbing a wall after the guards counted only the last member's rope; and at the wall by an old sally port each option from the second on did what the option below it should do (three "abandon the horses" options the game never handles pushed them out of step). Clothmakers' Street showed nothing by day but "leave": its shopfronts, which every other guild's street has, were switched off, so the clothmaker sold only at night, once woken, at a cost in reputation; they are open by day now. |
| `mods/fixes.c:268` | Saints taught by a cathedral, or by St. Cosmas or St. Damian, are learned |
| `mods/fixes.c:269` | Asking a cathedral about its patron saint was meant to teach the leader the saint, and St. Cosmas and St. Damian each teach the other; but the game gave it to no member: nobody learned the saint, and the fifth member was marked as knowing a formula instead. Now the saint is learned. |
| `mods/fixes.c:270` | Debt notes are where the merchant says |
| `mods/fixes.c:271` | A merchant who sends the party for debt notes names who holds them (the Medici or the Fugger), but the game hid them in another office above the marketplace, so breaking into the named one found nothing. They are now in the office named. |
| `mods/fixes.c:272` | The risk levels of alchemy formulae work the right way around |
| `mods/fixes.c:273` | Each formula is marked low, medium or high risk, and the risk decides how badly a failed batch at an inn, a village or a hideout goes: an explosion (the party hurt, damages to pay, no room at the inn for four months) or only a stink (one month). The game had it backwards: a failed low-risk formula exploded three times in four, a failed high-risk one only half the time. Now a failure explodes one time in four at low risk, one in three at medium risk and one in two at high risk. |
| `mods/fixes.c:274` | Quests from a cathedral are the bishop's, not the merchants' |
| `mods/fixes.c:275` | A cathedral's canon gave out quests as if for the town's merchants: the merchants paid for them (and gave no more quests of their own while one was open), the canon gave a new one at every visit, and the bishop's relic for a finished quest could be taken again and again. Now the quest counts as the bishop's, and his relic is the reward, given once. |
| `mods/fixes.c:276` | No quests to a purged pagan altar |
| `mods/fixes.c:277` | A pagan altar the party has destroyed vanishes from the overland view, but quests could still send the party there, and could then never be done (nor could their giver offer another). Such altars are no longer chosen; quests already sent to one go to the nearest altar still standing. |
| `mods/fixes.c:278` | The item taken from a tomb goes to the winner of the fight |
| `mods/fixes.c:279` | Taking the item a quest seeks from a tomb wakes its dead: winning that fight gave nothing (though the text says the party scrambles out with it) and losing it gave the item. Now the party gets the item when it wins. |
| `mods/fixes.c:280` | The lord pays for his quests |
| `mods/fixes.c:281` | In a town without a council, the lord of the fortress gives robber knights to hunt, but the game pays such a quest's reward only at a council's hall, so in such a town it could never be collected. The lord now pays it at an audience, as a council does. |
| `mods/fixes.c:282` | Armor and fire protection from potions and saints last a day; Hardarmor works as the manual says |
| `mods/fixes.c:283` | Hardarmor (+1 to +3 armor), St. Dunstan's (+1) and St. Patrick's (+2) blessings, St. Crispin's (+10 to the quality of everyone's armor) and the protection against fire of Firewall and St. Polycarp never wore off, and each new one added to the last. They now last about a day, as the manual says, and what piled up this way in a saved game goes back to normal. St. Charity, St. Erasmus and St. Pantaleon (and St. Polycarp) gave their protection against fire as armor instead: +32 or +48 armor for good (or nothing, with the fix "No invulnerability from fire protection" on); now the member gets fire protection 90 for a day, the same as St. Polycarp's. Hardarmor also does what the manual says: armor 20, 25 or 30 points better in quality and one or two levels thicker (the game gave 1 to 3 levels and no quality); a second used while one is still working keeps the stronger of the two. |
| `mods/fixes.c:284` | Great Power raises the weapon's quality |
| `mods/fixes.c:285` | According to the manual, Great Power raises the quality of the weapon by 14, 22 or 30 (up to 99), and quality is what makes blows hit harder. The original added those points to the member's damage and penetration bonuses instead: +14 penetration, or +1 damage and +6 penetration, or +1 damage and +14 penetration. Now it raises the quality of the weapon in hand (bows and guns too), for about a day. |
| `mods/fixes.c:286` | Saved games keep the party's standing with the Fugger, the Medici and the Hanse |
| `mods/fixes.c:287` | The companies' cards raise and lower the party's standing with them (if it, or the party's reputation in the town, is below zero, they give no quests), but a saved game did not keep it: after loading, it was whatever the previous game played had left, or zero after a restart. It is now kept in the save (the original game can still load such saves). |
| `mods/fixes.c:288` | A saint's help called in a battle wears off |
| `mods/fixes.c:289` | In the original, a saint called on in a battle raised attributes and skills for good (usually), and the price some saints ask (a lowered attribute) was never given back: the battle lost track of what was to wear off. Both now wear off, as with help called on outside a battle. |
| `mods/fixes.c:290` | Healing no longer takes a boost away |
| `mods/fixes.c:291` | Healing (Essence o'Grace, or a saint's) brings a wounded attribute part of the way back to its maximum, but the game also brought an attribute above its maximum part of the way down to it: a member whose Strength or Endurance stood above the maximum (Ironarm, New-wind, a saint) lost most of that boost to the potion meant to heal them. Healing now only raises an attribute, never lowers it. |
| `mods/fixes.c:292` | Teachers teach only up to their own ability |
| `mods/fixes.c:293` | The game's texts say a teacher cannot take a skill beyond the teacher's own ability (a line the game never shows), and the physician turns away a party that knows more than he does; yet a teacher of any ability took a skill to 99. Lessons now stop at the teacher's ability: a member who has reached it rests instead. |
| `mods/fixes.c:294` | Each weapon oil works with the weapons the manual names |
| `mods/fixes.c:295` | Deadly Blade works with every weapon but impact weapons and flails, Strongedge with blades and polearms, and Great Power with impact weapons, flails and handguns, as the manual and the potions' own labels say. The game gave every oil to whatever weapon the member struck with, bows and fists too. |
| `mods/fixes.c:296` | The two St. Pauls do what their texts say |
| `mods/fixes.c:297` | St. Paul the Apostle gave what the texts give St. Paul the Simple (all lost Strength and Endurance healed, half the Intelligence lost for a day, and his requirements: Virtue 70), and Paul the Simple gave a copy of St. Thomas Aquinas's help. The Apostle's own help (Speak Common, Latin, Read & Write, Charisma, Endurance, a little Strength healed; Virtue 19, as the manual says) was in the game's code but never used. Now each does what its text says. |
| `mods/fixes.c:298` | A saint's reputation bonus goes to the right town |
| `mods/fixes.c:299` | Called on in Prag, St. John Nepomuk raised the reputation of some unrelated place (or of no real place) instead of Prag's; St. Cecilia, called on away from any town (in the overland view), raised the reputation of no real place at all. Now Nepomuk's bonus goes to Prag and Cecilia's to the nearest town, as with the other saints. |
| `mods/fixes.c:300` | Saints do what their texts say |
| `mods/fixes.c:301` | For 43 saints, the help the game gives falls short of or strays from the saint's own description: St. Christopher, St. Drogo and St. Wilfrid never sped the party's travel (now faster travel for a week: fewer days on the road), nor St. Eustace through woods; St. Nicholas and St. Theodore gave Flails instead of Riding; St. Charity, St. Erasmus and St. Pantaleon gave the party no protection against fire; St. Lutgardis's ecstasies gave Endurance instead of costing it; St. George, St. Michael, St. Longinius, St. Maurice and St. Eligius never improved a weapon or armor; St. Crispin improved metal armor, St. Dunstan none; reputation went to the wrong lands or none ... Now each does what its text says (the new character sheet and the new prayer screen describe each one). The armor and fire saints need the fix "Armor and fire protection from potions and saints last a day". |
| `mods/fixes.c:302` | The bowyer sells bows at night too |
| `mods/fixes.c:303` | Woken at night, a bowyer opened a shop of the smiths' weapons and armor and no bows: the night card hands the shop the smiths' goods. It now sells what the bowyer sells by day. |
| `mods/fixes.c:304` | Confession always takes at least an hour |
| `mods/fixes.c:305` | The better the party's reputation in a town, the shorter a confession in its church: at a reputation of 110 it took no time, and from 120 it turned the clock back. It now takes at least an hour. |
| `mods/fixes.c:306` | An audience promised for tomorrow is kept |
| `mods/fixes.c:307` | A council's clerk or a lord's steward who "fits you into the audience schedule tomorrow" set up an appointment that was never actually made, and the next day the party had to ask again like anyone else. The audience is now certain the next day. |
| `mods/fixes.c:308` | Many blows at once on one fighter all count |
| `mods/fixes.c:309` | The wounds a fighter takes are added up until they are dealt, in a number that holds at most 127. Four heavy blows landing at once (a member mobbed by a pack) went past it and turned negative: the fighter regained Endurance and Strength instead of falling. The sum now stops at 127. |
| `mods/fixes.c:310` | No freeze when placing the party in a castle |
| `mods/fixes.c:311` | To place the party on some levels of a castle, the game draws rooms at random until one is not the gate room; when the gate room was the only one to draw, it drew for ever and the game froze. It now takes the first room in that case. |
| `mods/fixes.c:312` | A failed search for formulae at a university shuts that search, not the saints' |
| `mods/fixes.c:313` | Asking a university about alchemical formulae and failing marked the question about saints as asked instead: asking about saints was refused for a week, and the formulae could be asked about again at once. It now marks the question about formulae. |
| `mods/fixes.c:314` | An item given to the whole party is given once |
| `mods/fixes.c:315` | Where a card gives every member something (out of a dungeon, from footpads), each member who had one already got it twice: added to his stack, and again as a new stack. Now each gets it once. |
| `mods/fixes.c:316` | A mount's quality helps when fleeing on horseback |
| `mods/fixes.c:317` | Fleeing the Wild Hunt or a wolf pack on horseback adds the quality of the poorest rider's mount to the chance, but the game looked in the wrong place for the mounts and so added nothing. It now finds the mounts. |
| `mods/fixes.c:318` | A church tithe blesses the right member |
| `mods/fixes.c:319` | When the party gave a tenth in a town church, the Divine Favor meant first for the member best at Religion went to no member at all (the game took the skill's value for a member's number); and sneaking into a guild leader's house at night showed a garbled name instead of the member with the lowest Agility. Both now use the right member. |
| `mods/fixes.c:320` | Small mistakes in single cards |
| `mods/fixes.c:321` | Claiming friendship with a Hussite band succeeded one time in a hundred, whatever the chance shown. A potion thrown at footpads was never used up. The inn's stablemaster asked about selling the party's mounts only when it had none. A village Schulz said "Please accept this florin" and gave none. The Templars' writing desk taught formulae the reader already knew. The witches' ointment could not be used to escape the sabbat between waves (both its options were switched off). The tatzelwurm's death text came even when nobody was eaten. Crossing the Cavern of Fire and Hail under armor, and clearing the spider warehouse, healed the party, though their texts say it is hurt; and the warehouse's owner had no name of his own ("I am . Are ye hardy adventurers?"). When the party was paid for a robber knight, the Hanse's guide or the village's Schulz who had come along was said to leave by the toss of a coin (or even when he had never come), and then stayed on. A second fight with a town's gate guards cost no standing (the card kept the standing it had found on arrival), the docks said "You need a missile weapon" to a party short of the fare, trouble on the way to a town's grove at night led to the grove's night card even after day had come, and sneaking into a guild's quarters made the whole party practice once for each member. Each now does what its text says. |
| `mods/fixes.c:322` | Climbing a city wall at night uses the right chances |
| `mods/fixes.c:323` | Along a city's walls at night, the best climber's chance with a rope was a number left over from the last failed climb by everyone (0 the first time: he always fell), and the chance of everyone climbing without one compared each member with that number instead of taking the weakest (certain the first time). The first member was never chosen as the climber. Now the climber is the best of all, his chance is his own, and the chance of everyone climbing is the weakest member's. |
| `mods/fixes.c:324` | A guard skilled with impact weapons keeps the camp safer |
| `mods/fixes.c:325` | Each member on guard at camp lowers the risk of an attack, by one point more when skilled (30 or more) with a weapon; but the game forgot to count impact weapons (maces, hammers, axes). Now they count too. |
| `mods/fixes.c:326` | No group of enemies goes missing from a battle |
| `mods/fixes.c:327` | When setting up a battle, the game drops the groups of enemies that turned out empty, but a mistake dropped every group after the first empty one too: some battles had fewer enemies than they were built with. Now only the empty groups are dropped. |
| `mods/fixes.c:328` | A town on alert remembers the trouble |
| `mods/fixes.c:329` | After a fight with the guards at the gate (and for weeks after a rising) a town is on alert, during which a standing of -10 or worse is meant to make it hostile (the guards at the gate, the docks, the town hall) instead of only -75 or worse. The original's test for the alert was the wrong way around, so the alert never counted. Now it does. |
| `mods/fixes.c:330` | The friars' day jobs are offered |
| `mods/fixes.c:331` | Two of the day jobs a member can find from the inn, preaching friar and mendicant friar, are meant to be open except in towns of a certain kind (which no town is); a mistake in the test closed them everywhere. Now a member who qualifies can be offered them. |
| `mods/fixes.c:332` | Shots through smoke clouds are tested correctly |
| `mods/fixes.c:333` | Whether a shot passes through a cloud (Black Cloud, smoke) was judged wrongly: shots were stopped by clouds that were not in the way, or went through ones that were. Now each cloud on the field is tested against the shot's own line. |
| `mods/fixes.c:334` | Cards that test two attributes add both |
| `mods/fixes.c:335` | Three cards look for the member best at two attributes added together (a passage in a dungeon, one in a town, one in the countryside): the game took the first attribute twice instead, from whichever member it compared last. Now each member's two attributes are added and the best sum counts. |
| `mods/fixes.c:336` | No loot lost past 255 of a kind |
| `mods/fixes.c:337` | Piles of loot gathered after a battle add up identical items in a count that holds at most 255: past that, the rest was lost (after a battle with many arrows or bolts). Now what does not fit goes into another row. |
| `mods/fixes.c:338` | Castle corridor decorations in the right place |
| `mods/fixes.c:339` | When laying out a castle, the game picks a spot along each passage's wall to decorate, counted from the passage's far end instead of its start: all but one possible roll put the spot past the passage's end, in another room. Now the spot is on the passage. |
| `mods/fixes.c:340` | A hireling killed in battle doesn't later send away his replacement |
| `mods/fixes.c:341` | When a hireling died in a battle, the end of his contract and his "has left" notice were not cancelled: later they could send away whoever had taken his place in the party. Now they are cancelled after the battle. |
| `mods/fixes.c:342` | Esc in character creation doesn't reopen a childhood already chosen |
| `mods/fixes.c:343` | In character creation, Esc on the childhood's points went back to the list of childhoods, though the childhood was already taken (the character was 15): picking a row there then started the adult profession with the same number on the other list. Now Esc there leads on to the professions, as Esc on a profession's points does. |
| `mods/fixes.c:344` | The burnt-out keep lets the party go after searching it |
| `mods/fixes.c:345` | Searching the burnt-out ruin of a robber knight's keep ends "as you resume your journey...", yet the card came back to its first page, as if the party had just arrived. Now the party is back on the road, as the text says. |
| `mods/fixes.c:346` | A giver asks for a robber knight once |
| `mods/fixes.c:347` | A town's lord or council (and any giver) always names the robber knight nearest its town; asked again a week later it gave the party the same knight a second time, as a second quest with its own deadline, though the party already had the first. Now a giver who already asked the party for that knight repeats the request instead of adding another. |
| `mods/fixes.c:348` | Of two teachers of one skill, the better one teaches |
| `mods/fixes.c:349` | When the party hired two teachers of the same skill in a town (the university's and the alchemist's, say), the inn always used whichever was hired first, its fee and its chance of a lesson taking, even when the other was far better. Now a member training in that skill learns from the best of them (the highest ability; the cheaper of two equals). |
| `mods/fixes.c:350` | Ordinary hits make a sound |
| `mods/fixes.c:351` | In the original, a blow that hits without knocking the target back makes no sound at all: only misses, parries, knock-backs, shots and deaths are heard, so many blows that land pass in silence. Now an ordinary hit sounds like a blow (the game's own knock-back sound, or the hit sound of a custom sound pack). |
| `mods/fixes.c:1105` | The store room keeps at most 127 of one item. |
| `mods/fixes.c:1106` | The store room is full: it keeps 100 kinds of items. |
| `mods/fixes.c:1277` | abandon the horses |
| `mods/fixes.c:1659` | %s has learned all the teacher of %s can teach (ability %d), and rests instead. |
| `mods/fixes.c:1918` | claim friendship |
| `mods/fixes.c:2222` | You need a missile weapon |
| `mods/fixes.c:2288` | Not bought: nobody in the party has room for another kind of item. |
| `mods/fixes.c:2306` | Nobody in the party has room for another kind of item. |
| `mods/fixes.c:2510` | a hook (or { -2 }) for each fix, in the same order |

## The interface (ui/)

| Where | Text |
|---|---|
| `ui/alchemyicons.cpp:201` | and sometimes appears to make more gold |
| `ui/alchemyview.cpp:73` |  mixes; the whole party's ingredients count. |
| `ui/alchemyview.cpp:78` | to every batch |
| `ui/alchemyview.cpp:80` | The party's Philosopher's Stone, of quality  |
| `ui/alchemyview.cpp:80` |  to the chance of every batch, whoever mixes. Its picture shows the stage of the alchemists' Great Work its quality has reached:  |
| `ui/alchemyview.cpp:80` | . An alchemist (up to 25) or a university (up to 29, depending on the town) can make it better, for a price. |
| `ui/alchemyview.cpp:81` | The party has no Philosopher's Stone yet. An alchemist (up to quality 25) or a university (up to 29) makes one, for a price; its quality adds to the chance of every batch. |
| `ui/alchemyview.cpp:88` | Formulae known |
| `ui/alchemyview.cpp:93` |  knows no formulae yet. |
| `ui/alchemyview.cpp:116` |  batches set |
| `ui/alchemyview.cpp:137` | Mystic number %d, %s |
| `ui/alchemyview.cpp:164` | For a batch: at hand / needed |
| `ui/alchemyview.cpp:181` | The party's ingredients |
| `ui/alchemyview.cpp:210` | The party carries %d. |
| `ui/alchemyview.cpp:210` | The party has none. |
| `ui/alchemyview.cpp:232` | not enough for more |
| `ui/alchemyview.cpp:234` | Chance, one batch: %d%% |
| `ui/alchemyview.cpp:239` | %d (fewer batches: higher) + %d Int + %d Alchemy + %d Stone - %d mystic |
| `ui/alchemyview.cpp:243` | Choose a formula in the book. |
| `ui/alchemyview.cpp:249` | Start mixing |
| `ui/alchemyview.cpp:250` | Start mixing |
| `ui/alchemyview.cpp:256` | The mystic number: the higher, the harder the formula is to brew. The risk is what a failed batch can cause. At an inn, in a village or a hideout, one failure in two ends in a mishap: %s in %s it is an explosion (the party hurt, damages to pay, no room at the inn for four months), otherwise a stink (no room for a month). In a camp in the wild a failure only costs the ingredients. |
| `ui/alerts.cpp:52` |   Next (%d more)   |
| `ui/alerts.cpp:52` |   Continue   |
| `ui/alerts.cpp:101` | Y: yes     N: no |
| `ui/alerts.cpp:101` | Any key or click: go on |
| `ui/barterview.cpp:132` | Click to have %s pay from now on |
| `ui/barterview.cpp:186` | Nothing to sell. |
| `ui/barterview.cpp:186` | Nothing for sale today. |
| `ui/barterview.cpp:303` | None of the party's formulae uses what this shop sells. |
| `ui/barterview.cpp:330` | Not sold here:  |
| `ui/barterview.cpp:330` | ; the party has the rest. |
| `ui/barterview.cpp:330` | The party has enough for that already. |
| `ui/barterview.cpp:331` |  (more than the purse holds) |
| `ui/barterview.cpp:331` | ; they go to  |
| `ui/barterview.cpp:332` |  Not sold here (to find elsewhere):  |
| `ui/barterview.cpp:336` | A batch takes  |
| `ui/barterview.cpp:341` |  (the party has  |
| `ui/barterview.cpp:341` | ; not sold here) |
| `ui/barterview.cpp:343` | . +1 and +5 buy what one or five more batches need. |
| `ui/barterview.cpp:346` | Not enough for a batch |
| `ui/barterview.cpp:370` | Click again to sell them |
| `ui/barterview.cpp:370` | Sell spare gear |
| `ui/barterview.cpp:377` | %d item%s not equipped (weapons, armor, shields; not ammunition or throwing weapons), for about %s in all: the ones pulsing in the list.%s |
| `ui/barterview.cpp:378` |  Click again to sell them all. |
| `ui/barterview.cpp:383` | For the formula |
| `ui/barterview.cpp:393` | the mount this member rides |
| `ui/barterview.cpp:395` | The merchant pays  |
| `ui/barterview.cpp:395` | The merchant will not buy this |
| `ui/barterview.cpp:396` |  - more than the purse holds |
| `ui/barterview.cpp:400` | Point at a ware or one of your items to see it here. |
| `ui/barterview.cpp:415` | Shift+Buy: ten  -  Shift+Sell or All: the whole stack  -  purchases go to the member picked above |
| `ui/barterview.cpp:416` | Shift+Buy: ten  -  Shift+Sell or All: the whole stack  -  click a name to see their items |
| `ui/barterview.cpp:417` | Shift+Buy: ten  -  Shift+Sell or All: the whole stack  -  click a name's coin to make that member pay |
| `ui/battlelog.cpp:121` | , the armor holds |
| `ui/battlelog.cpp:121` | ; knocked back |
| `ui/battlelog.cpp:122` | , knocked back |
| `ui/battlelog.cpp:126` | , knocked back, misses  |
| `ui/battlelog.cpp:191` | <skill> changed from X to Y |
| `ui/battlelog.cpp:191` | Edged +3 (now 33) |
| `ui/battlelog.cpp:193` |  changed from  |
| `ui/battlelog.cpp:195` |  changed from %d to %d |
| `ui/battlelog.cpp:198` | %s %+d (now %d) |
| `ui/battlelog.cpp:225` | The battle is over: the party has fallen. |
| `ui/battlelog.cpp:225` | The battle is over: the enemies are beaten. |
| `ui/battlelog.cpp:225` | The battle is over. |
| `ui/battlelog.cpp:249` |  falls unconscious |
| `ui/battlelog.cpp:251` |  leaves the field |
| `ui/battlelog.cpp:255` | , the armor holds |
| `ui/battlelog.cpp:289` | Each blow will be listed here. |
| `ui/battlelog.cpp:334` | No battle yet: the battle log shows here, blow by blow, during each battle and after it. |
| `ui/battlelog.cpp:337` | The last battle |
| `ui/battlelog.cpp:338` | The battle before |
| `ui/battleoverlay.cpp:216` | Click: attack |
| `ui/battleoverlay.cpp:216` | No clear shot |
| `ui/battleoverlay.cpp:240` | Battles need fewer clicks |
| `ui/battleoverlay.cpp:241` | Click a member (on the field, or their box at the left), then an enemy to attack or a spot to walk to: that click gives the order. Space pauses and resumes the battle; while it is paused, the next member without an order is chosen for you.  A member with a bow, crossbow, gun or thrown weapon shoots when you click an enemy and keeps shooting: at the next enemy when theirs falls, moving for a clear line when a wall, a tree or someone stands in the way, and drawing their melee weapon only when an enemy is beside them. A click on the bow or sword on their box at the left has them fight hand to hand instead.  Against armor their weapon cannot pierce, members choose Vulnerable (aimed at the gaps in the armor) by themselves when that can get through.  Point at an order button in the right margin to see what it does. |
| `ui/battleoverlay.cpp:248` | All this is Darklands+; each part can be switched off, for the game's own ways, in F12 > Settings > Battles. F11 lists the battle's keys and clicks. |
| `ui/battleoverlay.cpp:309` | The nearest enemy is this way: click to look |
| `ui/battleoverlay.cpp:350` | a little hurt |
| `ui/battleoverlay.cpp:351` | gravely wounded |
| `ui/battleoverlay.cpp:360` | Melee rating %d, aim %d |
| `ui/battleoverlay.cpp:361` | Melee rating %d |
| `ui/battleoverlay.cpp:363` | The master of this place: once this enemy has fallen and the fight is over, the place is won. |
| `ui/battleoverlay.cpp:380` | %s (hardness %d) |
| `ui/battleoverlay.cpp:394` |  does half damage |
| `ui/battleoverlay.cpp:394` |  is mostly deflected |
| `ui/battleoverlay.cpp:414` | \xe2\x96\xb6\xe2\x96\xb6 fast forward  (Z) |
| `ui/battleoverlay.cpp:417` | >> fast forward  (Z) |
| `ui/bestiary.cpp:38` | Could not read DARKLAND.ENM. |
| `ui/bestiary.cpp:42` | DARKLAND.ENM has an unexpected size. |
| `ui/bestiary.cpp:90` | natural weapons |
| `ui/bestiary.cpp:100` | Kind: %s (picture group %s) |
| `ui/bestiary.cpp:103` | Enemies of this kind: %s |
| `ui/bestiary.cpp:106` | natural armor |
| `ui/bestiary.cpp:106` | natural armor |
| `ui/bestiary.cpp:107` | Body: %s, hardness %d (%s); legs: %s, hardness %d (%s); armor quality %d |
| `ui/bestiary.cpp:117` | Weapons that pierce its body armor: %s |
| `ui/bestiary.cpp:118` | a natural shield |
| `ui/bestiary.cpp:119` | Shield: %s, quality %d |
| `ui/bestiary.cpp:120` | a natural missile |
| `ui/bestiary.cpp:122` | Melee: %s, quality %d |
| `ui/bestiary.cpp:127` | The item list (DARKLAND.LST) could not be read. |
| `ui/bestiary.cpp:132` | Every enemy in the game, with its numbers as the game's data file has them (in a fight, dice rolls and the difficulty level also count). Click a name to see its armor and weapons. The numbers can be changed and saved into a mod folder; enemies of one kind share their numbers, so a change applies to every enemy of that kind. |
| `ui/bestiary.cpp:133` | Every enemy in the game, with its numbers as the game's data file has them (in a fight, dice rolls and the difficulty level also count). Point at a name to see its picture; click it to see its armor and weapons. |
| `ui/bestiary.cpp:136` | %d kind%s changed |
| `ui/bestiary.cpp:146` | Armor quality |
| `ui/bestiary.cpp:191` | (no picture) |
| `ui/bestiary.cpp:230` | Save the enemies into this mod folder |
| `ui/bestiary.cpp:234` | Could not write  |
| `ui/bestiary.cpp:235` | . Tick the mod (F12 > Mods) and start the game again. |
| `ui/bestiary.cpp:238` | Undo all changes##enm |
| `ui/cacheview.cpp:42` | The store room |
| `ui/cacheview.cpp:43` | The party's items: one store room shared by every inn (Darklands+). |
| `ui/cacheview.cpp:44` | The party's items kept in  |
| `ui/cacheview.cpp:44` | , for when it returns. |
| `ui/cacheview.cpp:49` | Everything in the party's pack goes into the store, except kept items (Keep on the character sheet) and potions. |
| `ui/cacheview.cpp:50` | Everything %s carries that is not equipped goes into the store. |
| `ui/cacheview.cpp:57` | The party's pack |
| `ui/cacheview.cpp:84` | No potions or spare items in the pack. |
| `ui/cacheview.cpp:84` | Carries nothing. |
| `ui/cacheview.cpp:84` | Nothing stored here. |
| `ui/cacheview.cpp:139` | Point at an item to see it here; F11 shows what you can do. |
| `ui/cacheview.cpp:140` | Point at an item to see it here. Click a name above the right list (or press C) for another member. |
| `ui/cardhints.cpp:18` | Not for about a week after a fruitless search here for the religious library. |
| `ui/cardhints.cpp:19` | Not for about a week after a fruitless search here for alchemical formulae. |
| `ui/cardhints.cpp:20` | Not for about three weeks after a sewer grate in this town defeated you. |
| `ui/cardhints.cpp:21` | Needs the bribe in your purse; not for two days after a guard recognized you, nor for a day after guards spotted you sneaking at the wall. |
| `ui/cardhints.cpp:22` | Needs a rope, and not after you have waited a day and come back to the walls. |
| `ui/cardhints.cpp:23` | Needs a rope. |
| `ui/cardhints.cpp:26` | Not for about three weeks after a sewer grate here defeated you. |
| `ui/cardhints.cpp:28` | Not for two days after a guard refused your bribe here, nor if you cannot pay it. |
| `ui/cardview.cpp:339` |  speaks for the party |
| `ui/cardview.cpp:339` |  (the leader) |
| `ui/cardview.cpp:339` | . A formula traded is  |
| `ui/cardview.cpp:340` | Click to choose this with %s speaking. |
| `ui/cardview.cpp:380` | not possible |
| `ui/cardview.cpp:505` | Nobody in the party knows a saint who can help here. |
| `ui/cardview.cpp:505` | Nobody in the party carries anything that helps here. |
| `ui/cardview.cpp:532` | knows it already |
| `ui/cardview.cpp:540` | knows the formula but carries none |
| `ui/cardview.cpp:541` | at the easiest difficulty, the leader need not know the saint |
| `ui/cardview.cpp:541` | at the easiest difficulty, the leader needs none |
| `ui/cardview.cpp:542` | knows this saint already |
| `ui/cardview.cpp:584` | Each member tries it |
| `ui/cardview.cpp:587` | the town's opinion of the party (its reputation here) |
| `ui/cardview.cpp:588` | the party's fame |
| `ui/cardview.cpp:594` | It rests on:  |
| `ui/cardview.cpp:595` | Whoever speaks for the party tries it: the best at it, unless you pick another (the arrow at the end shows each member's chance). |
| `ui/cardview.cpp:596` | Whoever speaks for the party tries it: the leader, unless you pick another (the arrow at the end shows each member's chance). |
| `ui/cardview.cpp:607` | Brings a teacher to the inn |
| `ui/cardview.cpp:611` | Hired here now: %s. |
| `ui/cardview.cpp:612` | Not possible now |
| `ui/cardview.cpp:619` | Not possible now |
| `ui/cardview.cpp:648` | nobody knows it yet |
| `ui/cardview.cpp:661` | nobody knows this saint |
| `ui/cardview.cpp:661` | nobody carries one |
| `ui/cardview.cpp:666` | Saints who help here |
| `ui/cardview.cpp:666` | Formulae for sale |
| `ui/cardview.cpp:666` | Items that help here |
| `ui/cardview.cpp:671` | Not possible now |
| `ui/cardview.cpp:675` | (click to choose who learns one; point at a formula there for its details) |
| `ui/cardview.cpp:676` | (click to choose who) |
| `ui/cardview.cpp:692` | ..., for 3 groschen. |
| `ui/citynav.cpp:135` | Market stalls: everyday goods to buy and sell. |
| `ui/citynav.cpp:136` | Foreign trader |
| `ui/citynav.cpp:136` | Pavilions of traders from distant lands: goods to buy and sell. |
| `ui/citynav.cpp:137` | Swordsmith's Lane: weapons, and the swordsmiths' guild. |
| `ui/citynav.cpp:138` | Armorer's Way: armor and shields, and the armorers' guild. |
| `ui/citynav.cpp:139` | The Square of Bowyers and Gunsmiths: bows, crossbows and guns, and their guild. |
| `ui/citynav.cpp:140` | The Street of the Blacksmiths: weapons, and the blacksmiths' guild. |
| `ui/citynav.cpp:141` | Soldiers' Road |
| `ui/citynav.cpp:141` | Shops and push-carts crowded with soldiers and camp followers, and the street's leaders. |
| `ui/citynav.cpp:143` | Tinkers' Square: hammers, maces, flails and polearms, and the artisans' guild. |
| `ui/citynav.cpp:144` | Clothmakers' Street: clothing and soft armor (padded, leather, studded leather, cuir bouilli), and the clothmakers' guild. |
| `ui/citynav.cpp:145` | Astrologists' Lane: formulae to buy or trade, a better Philosopher's Stone, lessons in Alchemy. |
| `ui/citynav.cpp:147` | Stalls with herbs and rare materials: alchemical ingredients. |
| `ui/citynav.cpp:148` | Wounds treated for a fee, lessons in Healing, alchemical ingredients bought and sold. |
| `ui/citynav.cpp:149` | Heavy coin changed into bank notes that weigh nothing, and back, for a fee. |
| `ui/citynav.cpp:150` | The Medici representative: bank notes, as at the Fugger bank. |
| `ui/citynav.cpp:151` | Hanseatic League |
| `ui/citynav.cpp:151` | The offices of the league of northern merchant towns: quests for pay. Asked politely, they judge the party by its standing with the Hanse; asked boldly, by its reputation here and its fame. Collecting a reward politely also raises that standing. |
| `ui/citynav.cpp:152` | The Fuggers' Leihhaus: a wide choice of everyday goods, and money for almost anything you sell. |
| `ui/citynav.cpp:153` | Mass (a little Divine Favor for everyone); a donation of a third of the purse (the leader's Divine Favor); the patron saint's story; an errand for the canon (a relic); sanctuary. |
| `ui/citynav.cpp:154` | Mass at set hours (Divine Favor for everyone, more with Religion and Latin); confession (the leader's Divine Favor, a few hours); a tithe of a tenth of the purse (everyone's Divine Favor filled); sanctuary when the town is against you. |
| `ui/citynav.cpp:155` | The monks' library (Religion, Latin) and prayers for Divine Favor. |
| `ui/citynav.cpp:156` | New saints and alchemical formulae to learn, rare alchemical materials, a better Philosopher's Stone. The staff expect Latin and a sharp mind. |
| `ui/citynav.cpp:157` | Nuns tend the sick: alms, a day's work beside them (Healing), wounds tended for a fee, or free of charge if they accept you. Open by day only. |
| `ui/citynav.cpp:158` | Meals and a bed, news and rumors, the stables, a store room; stay to study or train. |
| `ui/citynav.cpp:159` | Notices, gossip and news. |
| `ui/citynav.cpp:160` | The city council: an audience with the lord, prisoners in the dungeons. |
| `ui/citynav.cpp:161` | The city's prison. |
| `ui/citynav.cpp:162` | The city guard's headquarters: people to join the party, weapons training. |
| `ui/citynav.cpp:163` | The lord's fortress: an audience with the lord, prisoners in the dungeons. |
| `ui/citynav.cpp:164` | Boats to other towns, for a fee (each leaves on its own day). |
| `ui/citynav.cpp:165` | A dangerous slum: rumors, and a room for next to nothing. |
| `ui/citynav.cpp:166` | A quiet corner where the party can wait: an hour, three hours, or until nightfall. |
| `ui/citynav.cpp:167` | An alley to a stretch of the wall: a way out when you cannot use the gates. |
| `ui/citynav.cpp:168` | Leave the city by one of its gates. |
| `ui/citynav.cpp:170` | Quest givers |
| `ui/citynav.cpp:170` | Learning and healing |
| `ui/citynav.cpp:193` | arms and armor |
| `ui/citynav.cpp:193` | less violent |
| `ui/citynav.cpp:198` | other crafts |
| `ui/citynav.cpp:206` | city marketplace |
| `ui/citynav.cpp:206` | central city square |
| `ui/citynav.cpp:207` | crafts district |
| `ui/citynav.cpp:207` | central fortress |
| `ui/citynav.cpp:208` | crafts district |
| `ui/citynav.cpp:208` | side streets and alleys |
| `ui/citynav.cpp:209` | posted notices |
| `ui/citynav.cpp:294` | Go to %s: %s |
| `ui/citynav.cpp:358` | simply walk out |
| `ui/citynav.cpp:366` | leave the city |
| `ui/citynav.cpp:367` | out of the city |
| `ui/citynav.cpp:389` | The gate: choose how to get out, and the journey begins in the overland view. |
| `ui/citynav.cpp:407` | left the city. |
| `ui/citynav.cpp:408` | too many steps; stopped. |
| `ui/citynav.cpp:417` | something happened on the way. |
| `ui/citynav.cpp:417` | leave this place first. |
| `ui/citynav.cpp:425` | the way is blocked. |
| `ui/citynav.cpp:427` | cannot find the way from this card. |
| `ui/citynav.cpp:448` | leading merchant |
| `ui/citynav.cpp:453` | something happened on the way. |
| `ui/citynav.cpp:455` | something happened on the way. |
| `ui/citynav.cpp:455` | leave this place first. |
| `ui/citynav.cpp:465` | the marketplace by night: choose how to get past the guards to the offices upstairs (press G again there for the office). |
| `ui/citynav.cpp:466` | the marketplace by night: its stalls are shut until 5:00. |
| `ui/citynav.cpp:487` | not found in this city. |
| `ui/citynav.cpp:497` | Going to: %s. Press any key or click to stop. |
| `ui/citynav.cpp:527` | Walking to the city gate |
| `ui/citynav.cpp:527` | Walking to: %s |
| `ui/citynav.cpp:540` | Any key or click stops the walk. |
| `ui/citynav.cpp:657` | Point at a place to read about it here: what it offers, its quests, its teachers. |
| `ui/citynav.cpp:668` | Click the tile and the party walks there. |
| `ui/citynav.cpp:707` | Pick a place and the party walks there through the streets. Press any key or click to stop them. |
| `ui/citynav.cpp:709` | Night (%d:00): the marketplace's stalls and offices are closed, and the watch is out. |
| `ui/citynav.cpp:737` | Closed until 5:00 |
| `ui/citynav.cpp:737` | Closed at night: the marketplace's stalls and offices open at 5:00. Going there takes the party to the marketplace all the same. |
| `ui/citynav.cpp:741` | Turned away until %d %s |
| `ui/citynav.cpp:741` | The alchemists turned the party away; they will see you again from %d %s, %d:00. |
| `ui/citynav.cpp:742` | Library shut until %d %s |
| `ui/citynav.cpp:742` | The monks' library (where saints are learned) is shut to the party until %d %s, %d:00. |
| `ui/citynav.cpp:743` | Prayers again from %d %s |
| `ui/citynav.cpp:743` | The party prayed with the monks; they may again from %d %s, %d:00. |
| `ui/citynav.cpp:744` | Library shut until %d %s |
| `ui/citynav.cpp:744` | The university's library (where saints are learned) is shut to the party until %d %s, %d:00. |
| `ui/citynav.cpp:762` | Quest chance %d%% |
| `ui/citynav.cpp:763` | A merchant may offer a quest after you have bought or sold something. |
| `ui/citynav.cpp:764` | No quests until %d %s |
| `ui/citynav.cpp:765` | On a quest for them |
| `ui/citynav.cpp:766` | Your reward is waiting |
| `ui/citynav.cpp:767` | No quests: reputation too low |
| `ui/citynav.cpp:772` |  Until %d %s, %d:00. |
| `ui/citynav.cpp:780` | Your quest:  |
| `ui/citynav.cpp:780` | see the world map (M) |
| `ui/citynav.cpp:802` | Wares %dq, better than most |
| `ui/citynav.cpp:804` | Wares of %dq (most shops sell about 25q). |
| `ui/citynav.cpp:814` | Raises a Philosopher's Stone to |
| `ui/citynav.cpp:814` | No better stone than the party's |
| `ui/citynav.cpp:817` | What you learned |
| `ui/citynav.cpp:817` | The physician:  |
| `ui/citynav.cpp:817` | The alchemist:  |
| `ui/citynav.cpp:889` | A teacher to hire (faint: not available now) |
| `ui/citynav.cpp:890` | A quest of yours leads here |
| `ui/createview.cpp:109` | A new character |
| `ui/createview.cpp:110` | %s, called %s, age %d |
| `ui/createview.cpp:111` | %s, called %s |
| `ui/createview.cpp:133` | Name and gender |
| `ui/createview.cpp:134` | Type a name or pick a random one, then go on to the childhood. |
| `ui/createview.cpp:135` | Short name (the one the cards use) |
| `ui/createview.cpp:148` | Enter keeps it, Esc cancels. |
| `ui/createview.cpp:153` | On to the childhood |
| `ui/createview.cpp:154` | Cancel and return to the party |
| `ui/createview.cpp:157` | The family decides the starting attributes and skills, and which professions are open at first. Point at one to see what it brings (on the right); click to choose it. |
| `ui/createview.cpp:170` | Five years each. Those in dark letters are open now: click one to take it. Point at any to see what it needs and brings. |
| `ui/createview.cpp:201` | taken before (can be taken again for five more years) |
| `ui/createview.cpp:202` | No profession is open to this character now. |
| `ui/createview.cpp:205` | %d of %d EP (experience points) left. Use + and - on a row (- only takes back this step's points). A point costs 1 EP, 2 at 29+, 3 at 39+; an attribute can reach 40 at most. |
| `ui/createview.cpp:219` | %d EP a point |
| `ui/createview.cpp:226` | On to the profession |
| `ui/createview.cpp:229` | %d of %d EP left. A point costs 1 EP, 2 at 49+, 3 at 79+. Room: how many more points this profession still lets you add to the skill. |
| `ui/createview.cpp:258` | %s %d: a point costs %d EP now; this profession allows %d more. |
| `ui/createview.cpp:263` | Another profession (five more years) |
| `ui/createview.cpp:264` | Begin adventuring |
| `ui/createview.cpp:283` | %d EP for the attributes, then age %d. |
| `ui/createview.cpp:283` | %d EP for the skills, then age %d. |
| `ui/createview.cpp:286` | Over 30, aging lowers attributes (shown below). |
| `ui/createview.cpp:292` | This character qualifies, but the game lists at most 15 professions and this one comes after them. |
| `ui/createview.cpp:302` | Professions open at first:  |
| `ui/createview.cpp:306` | The character so far |
| `ui/createview.cpp:308` | Point at a childhood to see what it brings. |
| `ui/createview.cpp:308` | Point at a profession to see what it needs and brings. |
| `ui/createview.cpp:319` | Divine Favor |
| `ui/createview.cpp:365` | teaches: the points the five years give; room: how many more points skill EP can add afterwards. |
| `ui/createview.cpp:380` | Saints known:  |
| `ui/editor.cpp:50` | Start or load a game first. |
| `ui/editor.cpp:51` | Changes here apply to the running game at once. Save afterwards to keep them. |
| `ui/editor.cpp:58` | florins in bank notes |
| `ui/editor.cpp:60` | Philosopher's Stone quality |
| `ui/editor.cpp:67` | reputation in  |
| `ui/editor.cpp:80` | Name, age, crest and sprite |
| `ui/editor.cpp:87` | crest (A..O) |
| `ui/editor.cpp:101` | (an enemy's sprite: its colors below are its own shades) |
| `ui/editor.cpp:102` | Colors of the sprite and its portrait (battles and the party panel): |
| `ui/editor.cpp:112` | Attributes (current / maximum) |
| `ui/editor.cpp:123` | Saints known |
| `ui/editor.cpp:134` | Formulae known (each potion has three) |
| `ui/editor.cpp:150` | Items: quality and count |
| `ui/enemyview.cpp:47` | press a key or click to return to the battle |
| `ui/enemyview.cpp:66` | %d on the field, %d still fighting |
| `ui/enemyview.cpp:67` | still fighting |
| `ui/enemyview.cpp:80` |  (enchanted) |
| `ui/enemyview.cpp:85` |  (enchanted) |
| `ui/enemyview.cpp:88` | Natural armor |
| `ui/enemyview.cpp:89` | Natural weapons |
| `ui/enemyview.cpp:103` |  does half damage |
| `ui/enemyview.cpp:103` |  is mostly deflected |
| `ui/figures.cpp:32` | two-handed sword |
| `ui/figures.cpp:33` | great hammer |
| `ui/figures.cpp:34` | potion throw |
| `ui/figures.cpp:35` | throwing axe |
| `ui/figures.cpp:35` | throwing knife |
| `ui/figures.cpp:42` | every weapon |
| `ui/figures.cpp:42` | An original party sprite: it has an animation for every weapon. |
| `ui/figures.cpp:51` | Its own animations:  |
| `ui/figures.cpp:51` | . Any other weapon is shown with the nearest of these (a sword swung as an axe, a bow drawn as a crossbow). |
| `ui/figures.cpp:117` | Click a sprite (or use the arrow keys and Enter). The first four are the original party sprites; the others are enemy sprites. The 1st, 2nd and 3rd color buttons recolor them afterwards. A save with a member using an enemy sprite loads only in Darklands+ (the original game stops at the missing picture). |
| `ui/figures.cpp:163` | Keep the current sprite (Esc) |
| `ui/gamedata.cpp:59` | Goods merchant |
| `ui/gamedata.cpp:106` | Missile weapons |
| `ui/gamedata.cpp:107` | Alchemical ingredients |
| `ui/gamedata.cpp:112` | Missile weapon |
| `ui/gamedata.cpp:113` | Alchemical ingredient |
| `ui/gamedata.cpp:188` | Rufinus of G |
| `ui/gamedata.cpp:189` | Leonard of M |
| `ui/gamedata.cpp:189` | Nicolaus of A |
| `ui/gamedata.cpp:191` | Studded Lthr |
| `ui/gamedata.cpp:191` | Studded Leather |
| `ui/gamedata.cpp:192` | East. Black Bean |
| `ui/gamedata.cpp:192` | Eastern Black Bean |
| `ui/gamedata.cpp:192` | Phleqmatic Base |
| `ui/gamedata.cpp:192` | Phlegmatic Base |
| `ui/gamedata.cpp:192` | Melanc. Base |
| `ui/gamedata.cpp:192` | Melancholic Base |
| `ui/gamedata.cpp:193` | Essence o'Grace |
| `ui/gamedata.cpp:193` | Essence of Grace |
| `ui/gamedata.cpp:193` | Birth Confess. |
| `ui/gamedata.cpp:193` | Birth Confession |
| `ui/gamedata.cpp:193` | Illum. Gospel |
| `ui/gamedata.cpp:193` | Illuminated Gospel |
| `ui/gamedata.cpp:194` | S.Mary's Tears |
| `ui/gamedata.cpp:194` | St. Mary's Tears |
| `ui/gamedata.cpp:194` | S.Arnulf GreatSwrd |
| `ui/gamedata.cpp:194` | St. Arnulf's Greatsword |
| `ui/gamedata.cpp:194` | S.Catherine Pain |
| `ui/gamedata.cpp:194` | St. Catherine's Pain |
| `ui/gamedata.cpp:195` | S.Olaf Battleaxe |
| `ui/gamedata.cpp:195` | St. Olaf's Battleaxe |
| `ui/gamedata.cpp:195` | S.George GreatSwrd |
| `ui/gamedata.cpp:195` | St. George's Greatsword |
| `ui/gamedata.cpp:195` | S.Dunstan Hammer |
| `ui/gamedata.cpp:195` | St. Dunstan's Hammer |
| `ui/gamedata.cpp:196` | Spar of S.Erasmus |
| `ui/gamedata.cpp:196` | Spar of St. Erasmus |
| `ui/gamedata.cpp:196` | Staff of S.Patrick |
| `ui/gamedata.cpp:196` | Staff of St. Patrick |
| `ui/gamedata.cpp:196` | S.Hubert's Bow |
| `ui/gamedata.cpp:196` | St. Hubert's Bow |
| `ui/gamedata.cpp:197` | S.Edward's Ring |
| `ui/gamedata.cpp:197` | St. Edward's Ring |
| `ui/gamedata.cpp:197` | S.Emygdius Finger |
| `ui/gamedata.cpp:197` | St. Emygdius' Finger |
| `ui/gamedata.cpp:197` | S.Gabriel's Horn |
| `ui/gamedata.cpp:197` | St. Gabriel's Horn |
| `ui/gamedata.cpp:198` | S.Ita's Needle |
| `ui/gamedata.cpp:198` | St. Ita's Needle |
| `ui/gamedata.cpp:198` | S.Kessog Medallion |
| `ui/gamedata.cpp:198` | St. Kessog's Medallion |
| `ui/gamedata.cpp:198` | S.Odo's Testament |
| `ui/gamedata.cpp:198` | St. Odo's Testament |
| `ui/gamedata.cpp:199` | S.Odilia's Oil |
| `ui/gamedata.cpp:199` | St. Odilia's Oil |
| `ui/gamedata.cpp:199` | S.Raphael's Water |
| `ui/gamedata.cpp:199` | St. Raphael's Water |
| `ui/gamedata.cpp:199` | S.Swithbert's Foot |
| `ui/gamedata.cpp:199` | St. Swithbert's Foot |
| `ui/gamedata.cpp:200` | S.Willehad's Shoe |
| `ui/gamedata.cpp:200` | St. Willehad's Shoe |
| `ui/gamedata.cpp:200` | S.Thealeaus' Spoon |
| `ui/gamedata.cpp:200` | St. Thealeaus' Spoon |
| `ui/gamedata.cpp:203` | St.Christina, Astonish. |
| `ui/gamedata.cpp:203` | St. Christina the Astonishing |
| `ui/gamedata.cpp:203` | St.Gertrude o'Nivelles |
| `ui/gamedata.cpp:203` | St. Gertrude of Nivelles |
| `ui/gamedata.cpp:204` | St.Gregory Thaumaturg. |
| `ui/gamedata.cpp:204` | St. Gregory Thaumaturgus |
| `ui/gamedata.cpp:204` | St. Thalelaeus the Merciful |
| `ui/gamedata.cpp:205` | St.Victor o'Marseilles |
| `ui/gamedata.cpp:205` | St. Victor of Marseilles |
| `ui/gamedata.cpp:205` | St.Edward, Confessor |
| `ui/gamedata.cpp:205` | St. Edward the Confessor |
| `ui/gamedata.cpp:206` | St.Julian, Hospitaler |
| `ui/gamedata.cpp:206` | St. Julian the Hospitaller |
| `ui/gamedata.cpp:209` | Gerard of Cremona's |
| `ui/gamedata.cpp:209` | Gerard of C's |
| `ui/gamedata.cpp:209` | Gerard of Cremona's |
| `ui/gamedata.cpp:209` | Petrus Bonus' |
| `ui/gamedata.cpp:210` | Nicolas Flammel's |
| `ui/gamedata.cpp:210` | Raimundus Ls' |
| `ui/gamedata.cpp:210` | Raimundus Lullus' |
| `ui/gamedata.cpp:211` | Raimundus L's |
| `ui/gamedata.cpp:211` | Raimundus Lullus' |
| `ui/gamedata.cpp:211` | Arnald of V's |
| `ui/gamedata.cpp:211` | Arnald of Villanova's |
| `ui/gamedata.cpp:211` | Albertus Ms' |
| `ui/gamedata.cpp:211` | Albertus Magnus' |
| `ui/gamedata.cpp:212` | Albertus M's |
| `ui/gamedata.cpp:212` | Albertus Magnus' |
| `ui/gamedata.cpp:212` | Rob de Ketene's |
| `ui/gamedata.cpp:212` | John of Rupescissa's |
| `ui/gamedata.cpp:213` | John Rupesc.'s |
| `ui/gamedata.cpp:213` | John of Rupescissa's |
| `ui/gamedata.cpp:213` | Michael Scot's |
| `ui/gamedata.cpp:214` | Essence o'Grace |
| `ui/gamedata.cpp:214` | Essence of Grace |
| `ui/guide.cpp:28` | The party: Endurance, ... |
| `ui/guide.cpp:36` | Darklands has no tutorial: this page sums up how it works, with the advice players have long given one another. F11 (a Darklands+ addition) lists the keys and clicks of the current screen at any time; a right-click opens the game's menus. |
| `ui/guide.cpp:39` | Lines in this color depend on something Darklands+ adds (most can be switched off in F12 > Settings). |
| `ui/guide.cpp:59` | A new party has little money and light armor. Buy the best armor the purse allows before anything else (studded leather or mail if you can): a weapon whose penetration is below an armor's hardness is mostly turned aside (Battle, below). |
| `ui/guide.cpp:60` | The usual way to toughen a new party: walk a town's side streets and docks at night. Thugs and cutthroats attack; each fight raises weapon skills quickly, and their weapons and armor sell well (cloth armor sells for little). The worst the night watch does is fine you. Go back to the inn to rest between rounds. |
| `ui/guide.cpp:61` | +Darklands+ can make these night ambushes rarer (F12 > Mods: Night encounters in the streets); leave it as the original if you mean to train this way. |
| `ui/guide.cpp:62` | Quests (paid work) come from a town's merchants, foreign traders and pharmacists (at the marketplace), the Fugger, the Medici, the Hanse, the town council and the cathedral: robber knights to kill, items to fetch. The banks' and the council's pay best. Note down what you were asked to do and where. |
| `ui/guide.cpp:63` | +The left margin lists the party's quests, and the world map (M) marks where each one leads. |
| `ui/guide.cpp:64` | Rest at an inn when Strength is low, and set out with it full. In the wilds, camping costs nothing. |
| `ui/guide.cpp:65` | Keep the party's Virtue up and never attack a priest or a monk: the Church remembers. Cleared streets and good deeds raise the town's opinion of the party. |
| `ui/guide.cpp:66` | A good party has a leader with high Charisma who speaks the common tongue well, someone who reads Latin, and a woodsman (Woodwise) for the road. Nürnberg, in the middle of the land, is a good home town. |
| `ui/guide.cpp:67` | +Sell what nobody uses: in a shop, the Sell spare gear button sells at once every weapon, armor and shield that is not equipped and that the shop buys (not ammunition, throwing weapons or items marked Keep). |
| `ui/guide.cpp:68` | Save often in towns. |
| `ui/guide.cpp:69` | +Darklands+ also saves by itself on arriving at a town or any other place, and before a battle. |
| `ui/guide.cpp:71` | The party: Endurance, Strength, Divine Favor |
| `ui/guide.cpp:72` | Endurance (End) is breath and bruises: blows take it first and rest brings it back. At 0 a member falls unconscious. |
| `ui/guide.cpp:73` | Strength (Str) is real wounds: it falls with heavy blows and comes back slowly. At 0 the member is dead. Below half, keep that member out of the fight. |
| `ui/guide.cpp:74` | Divine Favor (DF) is spent calling on saints and comes back with prayer (and slowly by itself). |
| `ui/guide.cpp:75` | Each member's attributes and skills are on the character sheet (click the member in the party panel at the left, or press F1 ... F5). |
| `ui/guide.cpp:76` | +The bars on the party panel show End, Str and DF all the time, with numbers. |
| `ui/guide.cpp:77` | The party has room for four characters of your own: at an inn, "reconsider the composition of your party" creates a new one for an empty slot or dismisses one. A fifth slot is for companions some cards bring along for a while (a village's headman, a knight). |
| `ui/guide.cpp:78` | Each member carries their own items. Only equipped items (weapons, armor, shield) count toward a member's load, and a heavy load slows them in battle. |
| `ui/guide.cpp:79` | +With the shared pack (Darklands+), everything not equipped goes into one pack for the whole party, so nobody has to pass items between members. Each member keeps their equipped weapons and armor, their mount and their own potions. When the game needs a particular member to have something (lock picks at a lock, arrows before a battle, a horse), Darklands+ hands it to them from the pack first. |
| `ui/guide.cpp:80` | On a card, the test is made by the leader. |
| `ui/guide.cpp:80` | +Darklands+ has the member best at it take the test instead (F12 > Settings > Cards and towns: The best member speaks for the party), with the chance and that member's head beside the option; the arrow at the end of the chance shows every member's, and a click on one has that member speak instead. Whoever speaks also bears what comes of it, as the card's text says: wounds, a trap's lasting harm, a shrine's favor. On the alchemist's formula options you pick who speaks (each member with their own chance), since what is traded is learned by the one who speaks. In a battle or a dungeon, while unpaused, the leader deals with doors, chests and locks. |
| `ui/guide.cpp:82` | Money and reputation |
| `ui/guide.cpp:83` | 1 florin = 20 groschen; 1 groschen = 12 pfennigs. The purse is the party's; prices change with the town's opinion of you. |
| `ui/guide.cpp:84` | The party has a separate reputation in each town: good deeds there raise it, crimes and brawls lower it. A bad reputation brings trouble with the watch and higher prices. |
| `ui/guide.cpp:85` | A reputation runs from -99 to 99, in six named levels: hunted (-75 and below), wanted, suspected, unknown (-9 to 10), respected (11 to 50) and a local hero (above 50). Most deeds and saints move it by a random amount, and the further it is from 0 the likelier it stays as it is: at 50 one deed in two counts, at 90 one in ten. |
| `ui/guide.cpp:86` | Fame is the party's renown everywhere; it brings a new title every 50 points. |
| `ui/guide.cpp:87` | Party Info (F6, or the Party menu) shows the fame, the money and the reputation in the town the party is in; on its small map, point at a town to see the reputation there. |
| `ui/guide.cpp:87` | +Darklands+'s Party Info lists the reputation in every town at once. |
| `ui/guide.cpp:90` | +G opens Go to: a list of the town's places (shops, churches, the inn, the gates ...); pick one and Darklands+ takes the party there, choosing the town's menu entries for you. |
| `ui/guide.cpp:91` | The inn: a room for the night, or stay for days to rest, regain Strength, pray, brew, earn a little or study with a teacher. |
| `ui/guide.cpp:92` | +The inn's screen shows each member's day and what it costs; "Until rested" and "A week" spend the days for you, stopping when something changes. |
| `ui/guide.cpp:93` | The innkeeper keeps items for the party (store items, recover items stored here), in a store room of each town's own. |
| `ui/guide.cpp:94` | +With the shared store room (Darklands+), one store room serves every inn: what is left in one town is there in all of them (F12 > Journal > Stored lists it). |
| `ui/guide.cpp:95` | Teachers: a guild's masters (blacksmiths, swordsmiths, armorers, bowyers, artificers), the university, the monks, the physician and the alchemists can be asked to teach; a teacher found stays in town for a week and teaches at the inn, for a fee each day. |
| `ui/guide.cpp:96` | Each shop's goods have a quality, written like 29q: most shops' weapons and armor are about 25q, the best in the land near 38q. |
| `ui/guide.cpp:97` | +The right margin marks a town's notable shops (30q and better) with an icon and their quality; the world map (M) lists them all and finds where to buy a thing. In a shop, a weapon or armor is compared with what the member has equipped. |
| `ui/guide.cpp:98` | Almost every town has a monastery, where the monks teach saints, pray for the party and heal. Only some towns have a university, a cathedral, an alchemist, a physician, docks with boats or a bank. |
| `ui/guide.cpp:99` | +Under a town's name, the margin shows icons for what the town has that most towns lack (point at one to see what it offers); the world map (M) finds every town that has one. |
| `ui/guide.cpp:100` | Night falls at 19:00 and lasts until 5:00. At night the streets have the night watch, cutthroats lie in wait for a party leaving a side street (a stealthy, streetwise party slips past them), and gates may be closed. |
| `ui/guide.cpp:101` | +At night the margin shows "Night in the streets" with the party's chances. |
| `ui/guide.cpp:102` | Witches gather at Sabbats on set nights at places in the wilds. Now and then a card gives away a Sabbat's night or place (a captured villager, a dying cultist, a witch's letter). |
| `ui/guide.cpp:103` | +Once the party has heard of a Sabbat, it is listed among the quests and its place is marked on the world map (M) and in the overland view. A town that bars the party, or whose watch wants the party, has a sign beside its name on both. |
| `ui/guide.cpp:105` | Healing and rest |
| `ui/guide.cpp:106` | At an inn, a member who relaxes or regains Strength has full Endurance again the next morning. Regain Strength adds (the party's best Healing skill) / 15 Strength a day (at least 1). |
| `ui/guide.cpp:107` | Praying (at an inn) adds (Religion + Virtue) / 12 + 1 Divine Favor a day. |
| `ui/guide.cpp:108` | A wounded party should not travel far: an encounter on the road at low Strength can kill. |
| `ui/guide.cpp:111` | Alchemy is strong but slow to build up: each member knows formulae of their own, and a formula makes one potion. Alchemists sell formulae. |
| `ui/guide.cpp:112` | Each town's alchemist sells four formulae, the same for the whole game, and only the formulae of the first seven potions (Noxious Aroma to Sunburst) and one Thunderbolt formula are ever sold: the others are traded for or found. |
| `ui/guide.cpp:113` | +The character sheet's Formulae page, set to All formulae, names the nearest visited alchemist that sells each formula the party lacks; the margin ("Formulae sold here", with the prices) and the world map's tooltip on a town list what its alchemist sells. |
| `ui/guide.cpp:114` | The ingredients are bought from pharmacists, herbalists and alchemists; ingredients carried by any member count. |
| `ui/guide.cpp:115` | +In such a shop, the Formulae button lists the party's formulae; +1 or +5 beside one buys the ingredients for one or five more batches. |
| `ui/guide.cpp:116` | A batch's chance of success: 100 + the Philosopher's Stone's quality + Intelligence + Alchemy - the formula's mystic number. The 100 is less when more batches are made at once: 50 for four or five, 33 for six or seven, and so on. The party has one Philosopher's Stone at most (Party Info, F6, shows its quality). Brewing takes a day of alchemy at an inn or in camp. |
| `ui/guide.cpp:117` | A failed batch at an inn, in a village or a hideout ends in a mishap one time in two: an explosion (the party hurt, damages to pay, no room at that inn for four months) or a stink (no room for a month). The formula's risk (low, medium, high) makes it an explosion one time in four, three or two. In a camp in the wilds a failure only costs the ingredients: brew there when the chance is poor. |
| `ui/guide.cpp:118` | A potion is as good as the formula it was brewed with: each potion has three formulae, of grade 1, 2 and 3. |
| `ui/guide.cpp:119` | +The marks on a flask (one, two or three) show its grade (their look: F12 > Settings > Frames and clock), and the Formulae page lists every formula the party knows: who knows it, its ingredients against those carried, the batches they make and each alchemist's chance. |
| `ui/guide.cpp:122` | Saints are learned in a library (a town's monastery or university, or a monastery in the country) and called upon with Divine Favor, in a card's options or from the menu. Each saint helps with something different; calling on one takes Religion, Virtue and Divine Favor. |
| `ui/guide.cpp:123` | Each library teaches four saints, the same for the whole game. |
| `ui/guide.cpp:124` | +The character sheet's Saints page, set to All saints, names the libraries visited that teach each saint the party lacks, and says what each saint really does; the margin ("Saints taught here") and the world map say what a town's library teaches. |
| `ui/guide.cpp:125` | +Darklands+ shows the chance on the prayer screen and in a card's options. |
| `ui/guide.cpp:126` | A saint's help lasts about a day. Some saints ask a price for it, a loss of Perception, Intelligence, Agility or Charisma for that day (the saint's legend says so); the lost points come back when the help runs out. |
| `ui/guide.cpp:127` | +The margin's "Blessings and potions" shows what is at work on each member, and for how long; on the character sheet's Skills page a raised value is blue. |
| `ui/guide.cpp:130` | Battles run in real time; Space pauses. To give an order, select a member (click them, or press 1 to 5), pick the order from the menus (Std Attack, Vulnerable, Berserk, Parry, Use Missile, Walk towards, Halt and more), click the enemy or the spot it is for, then press Enter. |
| `ui/guide.cpp:131` | +With Easier battle controls (Darklands+), a click on an enemy (to attack it) or a spot (to walk there) gives the order at once, with no menu and no Enter, and if selecting the member paused the battle, it goes on; while the battle is paused, the next member without an order is selected. A shooter's box at the left has a bow or a sword on it: click it to choose whether a click on an enemy makes them shoot or fight hand to hand. Z fast-forwards. |
| `ui/guide.cpp:131` | +The right margin shows the selected member's orders as buttons, so the menus are not needed. |
| `ui/guide.cpp:132` | +With Improved combat (Darklands+), members fight on sensibly between orders: a shooter keeps shooting (at the nearest enemy in sight when the target falls), steps aside for a clear line, and draws a melee weapon once an enemy is close, or to help a party member nearby who is set upon by two or more or badly hurt; a member whose enemy falls turns on one close by. |
| `ui/guide.cpp:133` | Ctrl+click a member and the view follows them. |
| `ui/guide.cpp:133` | +The view scrolls when the pointer rests on its edge (the original needs Shift held there), and selecting a member does not move it: press their number twice to bring the view to them. |
| `ui/guide.cpp:134` | Armor has a hardness (soft, stiff, scale, mail, plate); a weapon whose penetration is above it pierces and wounds, one equal to it is halved, one below it is mostly turned aside. Short swords and daggers do little against plate. E (Enemy Info) shows each kind of enemy. |
| `ui/guide.cpp:135` | +Point at an enemy to see how your weapons fare against its armor; Enemy Info shows it for every kind of enemy on the field. |
| `ui/guide.cpp:136` | Skills grow with use in battle (and with lessons at an inn); when a fight ends, each gain shows in the member's box at the left. |
| `ui/guide.cpp:137` | +Darklands+ also shows each gain rising in gold over the fighter's head (+2 Edged). |
| `ui/guide.cpp:138` | Missile weapons need ammunition. A shooter needs a clear line to the target: party members, enemies and corners all block it, so move the shooter where the line is clear. |
| `ui/guide.cpp:139` | +Darklands+ can give the party endless ammunition (F12 > Settings > The party and its items). |
| `ui/guide.cpp:140` | After a fight in a street or a dungeon, walk the party out through an exit to go on. |
| `ui/guide.cpp:141` | In a castle, dungeon or mine the battle goes on after each fight; it ends when the master of the place has fallen and his fight is over, or when the party walks out (which the game counts as fleeing). |
| `ui/guide.cpp:142` | +The battle margin marks the master among the enemies, and the battle pauses when a group of enemies comes out to fight. With Enemies still waiting (F12 > Mods), it also counts the enemies not yet met, floor by floor, and says on which floor the master waits. |
| `ui/guide.cpp:145` | In the overland view, clicking a spot walks the party towards it. |
| `ui/guide.cpp:146` | +Ctrl+click travels there along the roads, around rivers (any key or click stops). M opens the world map: double-click a place to go there. |
| `ui/guide.cpp:147` | Each step costs time, and the road brings encounters. Roads are quickest, marsh and mountains slowest. The texts of St. Christopher, St. Drogo and St. Wilfrid promise faster travel, and St. Eustace's faster travel through woods, but in the original game they do not speed the party. |
| `ui/guide.cpp:148` | +The fix "Saints do what their texts say" (Darklands+, F12 > Settings > Fixes) makes these saints keep their texts' promise: faster travel for a week, so fewer days pass on the way. |
| `ui/guide.cpp:149` | Winter is hard on the road: from November to March a blizzard can catch the party in the open (in the high mountains from October to April). Keep going, call on a saint, or camp and wait it out. |
| `ui/guide.cpp:150` | Castles, monasteries, mines and the rest open their own card when the party steps on them. |
| `ui/guide.cpp:153` | Alt+S saves, with a comment, and Alt+L loads; each save is kept as a new one. Some places do not allow saving (in the middle of a card); save often in towns. |
| `ui/guide.cpp:154` | +Ctrl+S quick-saves; F9 or Ctrl+L loads the quick save. On Darklands+'s Save screen, clicking an old save replaces it with the new one, and the Load screen has a Delete button. |
| `ui/guide.cpp:155` | +Ironman (F12 > Settings > Preset, for the next new game): the game keeps one save and saves over it as you go (every card, each arrival, before each battle, on quitting). There is no loading back, and when the party's journey ends its save is deleted. |
| `ui/hourglass.cpp:322` | The game's own |
| `ui/hourglass.cpp:322` | Tower dial (about 1400) |
| `ui/hourglass.cpp:322` | Armillary, drawn anew |
| `ui/hourglass.cpp:322` | Copper astrolabe |
| `ui/hourglass.cpp:322` | Town clock (Prague, 1410) |
| `ui/hourglass.cpp:322` | The game's, drawn true |
| `ui/infoview.cpp:71` | %d (the next title at %d) |
| `ui/infoview.cpp:81` | Reputation here: %s (%d) |
| `ui/infoview.cpp:90` | Bank notes: %s |
| `ui/infoview.cpp:94` | Philosopher's Stone |
| `ui/infoview.cpp:96` | Quality %d: +%d to every batch |
| `ui/infoview.cpp:96` | None yet: an alchemist or a university makes one |
| `ui/infoview.cpp:101` | Reputation in the cities |
| `ui/infoview.cpp:103` | by reputation |
| `ui/infoview.cpp:142` | Last change:  |
| `ui/infoview.cpp:143` | , after choosing " |
| `ui/inventory.cpp:42` | East. Black Bean |
| `ui/inventory.cpp:43` | White Cinnabar |
| `ui/inventory.cpp:47` | Choleric Base |
| `ui/inventory.cpp:47` | Phleqmatic Base |
| `ui/inventory.cpp:48` | Sanguine Base |
| `ui/inventory.cpp:48` | Melanc. Base |
| `ui/inventory.cpp:68` | East. Black Bean |
| `ui/inventory.cpp:510` | Items can't change hands during a battle. |
| `ui/inventory.cpp:511` | Close the game's current screen (sheet, shop ...) to hand items over. |
| `ui/inventory.cpp:512` | Start or load a game first. |
| `ui/inventory.cpp:546` | Damage %d, penetration %d |
| `ui/inventory.cpp:547` | Time for a blow %d |
| `ui/inventory.cpp:548` | Time to %s %d |
| `ui/inventory.cpp:548` | draw and loose |
| `ui/inventory.cpp:552` | Skill needed %d: each point short costs 2 aim |
| `ui/inventory.cpp:553` | Strength %d to %d: below that, each point short costs 1 aim and hits are softer; above %d, hits are harder |
| `ui/inventory.cpp:554` | Strength %d: below that, each point short costs 1 aim and hits are softer |
| `ui/inventory.cpp:556` | Skill needed %d: each point short costs 2 melee rating (attack and defense hand to hand) |
| `ui/inventory.cpp:557` | Strength %d to %d: below that, each point short costs 3 melee rating and blows hit softer; above %d, each point adds 2 melee rating and blows hit harder |
| `ui/inventory.cpp:558` | Strength %d: below that, each point short costs 3 melee rating and blows hit softer |
| `ui/inventory.cpp:560` | Lower times are quicker. |
| `ui/inventory.cpp:571` | too weak by  |
| `ui/inventory.cpp:572` | suits (strong enough to hit harder) |
| `ui/inventory.cpp:575` | melee rating |
| `ui/inventory.cpp:587` | Covers the vitals (body and arms) |
| `ui/inventory.cpp:587` | Covers the legs |
| `ui/inventory.cpp:589` | Hardness %d of 5 (%s) |
| `ui/inventory.cpp:596` | Deflects most of the blow (penetration below %d): |
| `ui/inventory.cpp:596` | Halves the blow (penetration %d): |
| `ui/inventory.cpp:596` | Pierced (penetration above %d): |
| `ui/inventory.cpp:606` | A blow that pierces also wounds Strength; a good hit gets through more easily. |
| `ui/inventory.cpp:607` | Quality: every 10 points take 1 off each blow's damage. |
| `ui/inventory.cpp:611` | Adds to its bearer's defense, more the better it is made; unusable with a two-handed weapon. |
| `ui/inventory.cpp:615` | %s shield at %dq: %d against blows, %d against shots and throws |
| `ui/inventory.cpp:616` | No shield counts as -5, and a flail's blow passes any shield. Hand to hand, the chance to hit is 50 + 2/3 of the attacker's melee rating minus (the defender's melee rating + the shield); against a shot or throw, the defense is the shield plus twice Agility (less under a load). |
| `ui/inventory.cpp:622` | How it works: %s |
| `ui/itemedit.cpp:32` | Could not read DARKLAND.LST. |
| `ui/itemedit.cpp:49` | Every item of the game. Change names, weight, quality, rarity and value here, then save the list into a mod folder; it takes effect when the game is started again with that mod ticked. |
| `ui/itemedit.cpp:51` | Every item of the game, with its weight, usual quality, rarity and value (to change them for a mod, choose a preset other than Full Original in F12 > Settings). |
| `ui/itemedit.cpp:127` | Save the list into this mod folder |
| `ui/itemedit.cpp:131` | Could not write  |
| `ui/itemedit.cpp:132` | . Tick the mod (F12 > Mods) and start the game again. |
| `ui/itemedit.cpp:135` | Undo all changes |
| `ui/journal.cpp:204` | Goods merchant |
| `ui/journal.cpp:358` | the countryside |
| `ui/journal.cpp:416` | the countryside |
| `ui/journal.cpp:419` | A made-up text, number , to try the chronicle's days and places. It runs on a little so that it wraps over two lines in the window....leave. |
| `ui/journal.cpp:429` | The chronicle is off: tick both boxes above (the Full Original preset keeps it off). |
| `ui/journal.cpp:431` | Hide the list of days |
| `ui/journal.cpp:431` | Show the list of days |
| `ui/journal.cpp:434` | %d text%s read in this game |
| `ui/journal.cpp:437` | Save as a text file |
| `ui/journal.cpp:451` | Could not write the file. |
| `ui/journal.cpp:464` | Every text the cards show is kept here, with the date, the place and the choice you made. |
| `ui/journal.cpp:464` | Nothing found. |
| `ui/journal.cpp:512` | < Day before |
| `ui/journal.cpp:516` |  (matching texts only) |
| `ui/journal.cpp:594` | a complete idiot: worse than none |
| `ui/journal.cpp:597` | can raise a Philosopher's Stone to  |
| `ui/journal.cpp:598` | no better Philosopher's Stone than the party's |
| `ui/journal.cpp:628` | Start or load a game first: each game has its own notes. |
| `ui/journal.cpp:634` | Your own notes for this game: clues, plans, who wanted what. Kept as darklands_notes_%u.txt. |
| `ui/journal.cpp:783` | %s  (%d wares, noted %s) |
| `ui/journal.cpp:862` | sold here (expected) |
| `ui/journal.cpp:882` | Start or load a game first. |
| `ui/journal.cpp:883` | Available after the battle. |
| `ui/journal.cpp:890` | What the party has left with innkeepers ("store items with the innkeeper" at a town's inn): one store room, the same at every inn (the Darklands+ switch "One store room shared by every inn" in F12 > Settings > New screens). |
| `ui/journal.cpp:891` | What the party has left with innkeepers ("store items with the innkeeper" at a town's inn), and where.  |
| `ui/journal.cpp:891` | The original game shows a store room only while the party is in it. |
| `ui/journal.cpp:920` | Nothing stored anywhere. |
| `ui/journal.cpp:926` |  kind of item |
| `ui/journal.cpp:926` |  kinds of items |
| `ui/journal.cpp:927` | Every inn's store room  -   |
| `ui/journal.cpp:958` | The quests and every town (its shops, the wares noted, where to buy an item) are on the world map (M). |
| `ui/journal.cpp:961` | Note each town's shops and wares |
| `ui/journal.cpp:973` | The Honor Roll of Glory: the four most famous parties. MicroProse drew it and wrote the code for it, but never finished it (the manual mentions a Hall of Fame, the game's README says there is none). A party enters it when its journey ends, or now with the button. |
| `ui/journal.cpp:977` | Show the Honor Roll of Glory |
| `ui/journal.cpp:981` | Enter the party now |
| `ui/journal.cpp:981` | The party is on the roll. |
| `ui/journal.cpp:981` | The party is not yet famous enough to be among the four best (or the roll could not be saved). |
| `ui/journal.cpp:1015` | Where to buy |
| `ui/journal.cpp:1018` | Nothing noted yet: the wares of each shop are noted as the party sees them. |
| `ui/journal.cpp:1024` | Show %s on the world map |
| `ui/journal.cpp:1027` | Alchemical ingredients |
| `ui/journal.cpp:1028` | Bought ingredients are always quality 25. The nearest town first%s. |
| `ui/journal.cpp:1028` |  (for now by name: the party's position is not known) |
| `ui/journal.cpp:1032` | Nearest town |
| `ui/journal.cpp:1033` | Also sold at |
| `ui/journal.cpp:1049` | not seen in a shop yet |
| `ui/journal.cpp:1060` | Any other ware |
| `ui/journal.cpp:1077` | Not seen in any shop yet. |
| `ui/journal.cpp:1095` | Type a name: every town seen selling it, the nearest first. |
| `ui/levelmap.cpp:295` | The whole level  (floor %d) |
| `ui/levelmap.cpp:295` | The level, as far as the party has seen it  (floor %d) |
| `ui/levelmap.cpp:304` | enemies seen |
| `ui/levelmap.cpp:306` | arrows: who targets whom |
| `ui/levelmap.cpp:309` | stairs, ways out |
| `ui/levelmap.cpp:310` | not yet revealed |
| `ui/levelmap.cpp:311` | Click the floor to walk the party there. Tab or Esc closes the level map; the game is paused until then. |
| `ui/levelmap.cpp:311` | Tab or Esc closes the level map; the game is paused until then. |
| `ui/lodgeview.cpp:161` | Camping in the wild |
| `ui/lodgeview.cpp:161` | Staying here |
| `ui/lodgeview.cpp:182` | The party lives off the land, and the lord may hear of it: after %d day%s the risk is %d in 10. |
| `ui/lodgeview.cpp:183` | The room: %s a day.%s |
| `ui/lodgeview.cpp:183` |    Days stayed:  |
| `ui/lodgeview.cpp:211` | %d now, %d%% a day for +1, up to %d; %s a day |
| `ui/lodgeview.cpp:214` | Endurance back to full ( |
| `ui/lodgeview.cpp:214` | Already rested: earning would be better |
| `ui/lodgeview.cpp:214` | Already rested |
| `ui/lodgeview.cpp:216` | Divine Favor  |
| `ui/lodgeview.cpp:217` | Endurance stays down ( |
| `ui/lodgeview.cpp:235` | Regain Endurance |
| `ui/lodgeview.cpp:235` | Regain Strength |
| `ui/lodgeview.cpp:235` | Pray for Divine Favor |
| `ui/lodgeview.cpp:235` | Alchemy work |
| `ui/lodgeview.cpp:235` | Earn a little money |
| `ui/lodgeview.cpp:235` | Guard the camp |
| `ui/lodgeview.cpp:235` | Train or study |
| `ui/lodgeview.cpp:236` | Endurance is full: earning money is the better day. |
| `ui/lodgeview.cpp:236` | Already at full Strength. |
| `ui/lodgeview.cpp:236` | Divine Favor is full. |
| `ui/lodgeview.cpp:236` | Knows no formula to make. |
| `ui/lodgeview.cpp:236` | Only at an inn in a town. |
| `ui/lodgeview.cpp:236` | Only when camping in the wild. |
| `ui/lodgeview.cpp:236` | No teachers retained in this town. |
| `ui/lodgeview.cpp:237` | Rest: Endurance comes back to full. Nothing else is gained. |
| `ui/lodgeview.cpp:237` | Rest and care: Strength comes back; the better the party's Healing, the more (Endurance too). |
| `ui/lodgeview.cpp:238` | Prayer: Divine Favor comes back; the better Religion and Virtue, the more. Endurance does not come back that day. |
| `ui/lodgeview.cpp:238` | Make potions from a known formula (the ingredients are used up). Endurance does not come back that day. |
| `ui/lodgeview.cpp:239` | Work in the town: the job that pays this member best, by their attributes and skills (wounds lower the pay; a bad name in the town too). Endurance does not come back that day. |
| `ui/lodgeview.cpp:240` | Keep watch and find food: the camp is safer. |
| `ui/lodgeview.cpp:240` | Lessons from a teacher hired in this town, for a fee each day. Endurance does not come back that day. |
| `ui/lodgeview.cpp:266` | none retained |
| `ui/lodgeview.cpp:280` | learned all (%d) |
| `ui/lodgeview.cpp:281` | %d%%  \xc2\xb7  to %d  \xc2\xb7  %s  \xc2\xb7  %d day%s |
| `ui/lodgeview.cpp:292` |  At an inn a failed batch can blow up; in a camp in the wild it cannot. |
| `ui/lodgeview.cpp:300` | The teacher of %s knows %d; %s has %d already: nothing more to learn here. |
| `ui/lodgeview.cpp:302` | Teacher of %s (skill %d), %d more day%s hired. Each day: %d%% chance of +1 (about one point in %d day%s)%s. %s has %d.%s |
| `ui/lodgeview.cpp:303` | , up to the teacher's skill |
| `ui/lodgeview.cpp:304` |  (The best of the teachers of it hired here teaches.) |
| `ui/lodgeview.cpp:304` |  (The one hired first teaches.) |
| `ui/lodgeview.cpp:306` | No teachers retained in this town: ask a guild's masters, the university, the monks, the physician or an alchemist (with the Darklands+ switch "Teachers" on, a lectern marks the option on their cards); a teacher stays a week. |
| `ui/lodgeview.cpp:307` | Point at a choice to see what it does. |
| `ui/lodgeview.cpp:329` | In the purse |
| `ui/lodgeview.cpp:331` | Food: from the land |
| `ui/lodgeview.cpp:331` | Room and lessons |
| `ui/lodgeview.cpp:349` | Until rested |
| `ui/lodgeview.cpp:351` | The purse cannot pay for a day. Members set to Earn a little money may make up the difference; otherwise, leave. |
| `ui/lodgeview.cpp:352` | Days pass until no one is regaining Strength or praying (at most 14; it stops if anything happens). |
| `ui/lodgeview.cpp:352` | Nobody is short of Strength, regaining it or praying. |
| `ui/lodgeview.cpp:353` | Seven days, each member doing what was chosen (it stops if anything happens, or when a day cannot be paid). |
| `ui/lodgeview.cpp:362` | A day in camp |
| `ui/lodgeview.cpp:362` | A day at the inn |
| `ui/lodgeview.cpp:364` | , until rested |
| `ui/lodgeview.cpp:369` | Click or press a key to skip to the end |
| `ui/lodgeview.cpp:369` | Click or press a key to skip |
| `ui/lootview.cpp:61` |  in cash (added to the purse). |
| `ui/lootview.cpp:61` | No money found. |
| `ui/lootview.cpp:76` | The party's pack |
| `ui/lootview.cpp:103` | No potions or spare items in the pack. |
| `ui/lootview.cpp:103` | Carries nothing. |
| `ui/lootview.cpp:103` | Nothing left in the pile. |
| `ui/lootview.cpp:160` | A shop pays about  |
| `ui/lootview.cpp:163` | Point at an item to see it here; F11 shows what you can do. |
| `ui/lootview.cpp:164` | Point at an item to see it here. Click a name above the right list (or press D) for another member. |
| `ui/mapmsg.cpp:42` | press a key or click to go on |
| `ui/marchview.cpp:44` | Marching order |
| `ui/marchview.cpp:45` | The first leads the way; in single file the others follow in this order. |
| `ui/margins.cpp:68` | In the wilds |
| `ui/margins.cpp:69` | Traveling, near  |
| `ui/margins.cpp:189` | University: new saints and alchemical formulae to learn, rare alchemical materials, a better Philosopher's Stone, teachers to hire (it expects Latin and a sharp mind). |
| `ui/margins.cpp:190` | Cathedral: donations for Divine Favor (and maybe Virtue), an errand for the canon (a relic), the patron saint's story, sanctuary. |
| `ui/margins.cpp:191` | An alchemist: formulae to buy or trade, a better Philosopher's Stone, lessons in Alchemy. |
| `ui/margins.cpp:192` | A physician: wounds treated for a fee, lessons in Healing, alchemical ingredients bought and sold. |
| `ui/margins.cpp:193` | Docks: boats to other towns, for a fee (each boat leaves on its own day). |
| `ui/margins.cpp:194` | A bank (Fugger or Medici): heavy coins changed into a bank note that weighs nothing, and back, for a fee; quests for the bold. |
| `ui/margins.cpp:196` | armor and shields |
| `ui/margins.cpp:196` | bows, crossbows and guns |
| `ui/margins.cpp:197` | hammers, maces, flails and polearms (pikes, halberds, spears, staves) |
| `ui/margins.cpp:198` | clothing and soft armor (padded, leather, studded leather, cuir bouilli) |
| `ui/margins.cpp:236` | the party (best  |
| `ui/margins.cpp:253` | %s: %s to %d |
| `ui/margins.cpp:275` | Teachers here: a lesson a day at the inn, for a fee |
| `ui/margins.cpp:281` | Would teach %s |
| `ui/margins.cpp:282` | Nobody in the party would learn anything |
| `ui/margins.cpp:320` | Notable shops |
| `ui/margins.cpp:320` | Notable shops |
| `ui/margins.cpp:322` | %s: %s of %dq (most shops sell about 25q). |
| `ui/margins.cpp:327` | St.Edward, Confessor |
| `ui/margins.cpp:333` | Saints taught here |
| `ui/margins.cpp:346` |  (someone in the party knows this saint):  |
| `ui/margins.cpp:346` |  (nobody in the party knows this saint):  |
| `ui/margins.cpp:361` | Formulae sold here |
| `ui/margins.cpp:380` | Someone in the party knows this formula. |
| `ui/margins.cpp:380` | Nobody in the party knows this formula. |
| `ui/margins.cpp:399` | Teachers: some at the inn now |
| `ui/margins.cpp:400` | Teachers: %d here |
| `ui/margins.cpp:401` | Teachers: none to be had now |
| `ui/margins.cpp:404` | At the inn now:  |
| `ui/margins.cpp:404` |  Hired teachers give lessons at the inn: set a member to "Train or study" there, a lesson a day, for a fee.   |
| `ui/margins.cpp:511` | Reputation %d |
| `ui/margins.cpp:525` | Changes seen since this game was loaded: |
| `ui/margins.cpp:528` | the overland view |
| `ui/margins.cpp:528` | , after choosing " |
| `ui/margins.cpp:533` | The town hates the party: guards, shopkeepers and its people will turn on you. |
| `ui/margins.cpp:533` | The town distrusts the party: expect cold welcomes and worse. |
| `ui/margins.cpp:533` | The town is wary of the party. |
| `ui/margins.cpp:533` | The party is well thought of here. |
| `ui/margins.cpp:533` | The town's opinion of the party (rises with good deeds here, falls with crimes). |
| `ui/margins.cpp:547` | The town counts as hostile: its gates and people turn the party away. |
| `ui/margins.cpp:548` | The watch is after the party: guards who recognize the party try to arrest it, and talking or bribing your way out is harder. |
| `ui/margins.cpp:559` | Rebels and the government's men are hiring in this town. The news (the posted notices and gossip) and its rumors of special jobs lead to them; so does the guild where the agitators meet. |
| `ui/margins.cpp:571` | Preparing for war |
| `ui/margins.cpp:572` | Prices are a quarter higher, the gate turns travelers away and patrols watch the walls; Soldiers' Road and its market are open, and few boats take passengers. |
| `ui/margins.cpp:574` | Prices are 50% higher, food is short and the docks are barricaded; the gate turns travelers away and patrols watch the walls. Soldiers' Road and its market are open. |
| `ui/margins.cpp:599` | The party has goods stored here (F12 > Journal > Stored). |
| `ui/margins.cpp:606` | Blessings and potions |
| `ui/margins.cpp:624` | No blessings or potions at work. |
| `ui/margins.cpp:633` | C: camp here |
| `ui/margins.cpp:634` | Click (or press C) to make camp in the wild: rest, heal, brew potions, pray. Someone should guard the camp. |
| `ui/margins.cpp:638` | Go to a place here |
| `ui/margins.cpp:638` | G: go to a place here |
| `ui/margins.cpp:644` | Click (or press G) to open Go to: a list of the places in this town; pick one and the party walks there. |
| `ui/margins.cpp:693` | None taken on. |
| `ui/margins.cpp:725` | Travel there |
| `ui/margins.cpp:742` | ... %d more on the world map (M) |
| `ui/margins.cpp:758` | Night in the streets |
| `ui/margins.cpp:762` | almost always |
| `ui/margins.cpp:763` | about %d time%s in 10 |
| `ui/margins.cpp:764` | From 19:00 to 5:00 the town is dark. Each move through its streets may run into the night watch (more often on the main street, and where the town is on its guard). When leaving a side street, the party slips past cutthroats %s (depending on the party's Stealth and Streetwise)%s. An inn is the safe place to wait for morning. |
| `ui/margins.cpp:767` | , as set in F12 > Mods (Night encounters in the streets) |
| `ui/margins.cpp:807` | The canonical hours, as people in the game tell the time: Matins 0-2, Lauds 3-5, Prime 6-8, Terce 9-11,  |
| `ui/margins.cpp:807` | Sexts 12-14, Nones 15-17, Vespers 18-20, Compline 21-23.  |
| `ui/margins.cpp:809` | It is night (from 19:00 to 5:00): town gates may be shut, and the streets have the watch and cutthroats. |
| `ui/margins.cpp:809` | It is day (from 5:00 to 19:00). |
| `ui/margins.cpp:836` | %s in bank notes |
| `ui/margins.cpp:850` | F11: the keys and clicks of this screen. |
| `ui/margins_battle.cpp:80` | The battle runs again (the key: Space). Any order being given is given first. |
| `ui/margins_battle.cpp:81` | This member's order is given; the battle stays paused, to give others theirs (the key: Enter). |
| `ui/margins_battle.cpp:82` | Every kind of enemy on the field: attributes, weapons, armor (the key: E). |
| `ui/margins_battle.cpp:83` | Then click a spot: the member walks there. |
| `ui/margins_battle.cpp:84` | Then click a spot: the member runs there without stopping to fight or striking back (to get out of a melee). |
| `ui/margins_battle.cpp:85` | The member stops where they are and waits. |
| `ui/margins_battle.cpp:86` | The party moves together as a group. |
| `ui/margins_battle.cpp:87` | The party moves in single file, the leader first. |
| `ui/margins_battle.cpp:88` | Open or shut the door beside the member. |
| `ui/margins_battle.cpp:89` | Take the stairs beside the member to another floor. |
| `ui/margins_battle.cpp:90` | Open the chest beside the member. |
| `ui/margins_battle.cpp:91` | Pick the lock beside the member. |
| `ui/margins_battle.cpp:92` | Dissolve the lock beside the member with an Eater-Water potion. |
| `ui/margins_battle.cpp:93` | Disarm the trap on the chest beside the member. |
| `ui/margins_battle.cpp:94` | The whole party gives up the fight. |
| `ui/margins_battle.cpp:95` | Search the fallen for their belongings. |
| `ui/margins_battle.cpp:96` | Leave the battlefield by the way out beside the member. |
| `ui/margins_battle.cpp:97` | Stop giving this member an order (the key: Esc); the battle stays paused. |
| `ui/margins_battle.cpp:98` | Then click an enemy: a standard attack, balanced between blows and guard. |
| `ui/margins_battle.cpp:99` | Then click an enemy: blows aimed at the gaps in the armor (1 to 4 more penetration), but slower and a little less likely to land. |
| `ui/margins_battle.cpp:100` | Then click an enemy: quick, furious blows without thought of defense. |
| `ui/margins_battle.cpp:101` | Then click an enemy: the member mostly defends, hard to hit, striking back only now and then. |
| `ui/margins_battle.cpp:102` | Then click an enemy: shoot or throw at it with the missile weapon (a clear line needed). |
| `ui/margins_battle.cpp:135` | <name> finished |
| `ui/margins_battle.cpp:165` | does not end |
| `ui/margins_battle.cpp:174` | Enemies not yet met |
| `ui/margins_battle.cpp:176` | No one else waits in this place. |
| `ui/margins_battle.cpp:180` | No one on this floor (%d). |
| `ui/margins_battle.cpp:184` | %sfloor %d: %d |
| `ui/margins_battle.cpp:187` | Elsewhere: %s. |
| `ui/margins_battle.cpp:190` | The master of this place waits on this floor. |
| `ui/margins_battle.cpp:191` | The master of this place waits on floor %d. |
| `ui/margins_battle.cpp:210` | Back to the chosen speed |
| `ui/margins_battle.cpp:210` | Back to the chosen speed (Z) |
| `ui/margins_battle.cpp:210` | Fast forward |
| `ui/margins_battle.cpp:210` | Fast forward (Z) |
| `ui/margins_battle.cpp:254` | a little hurt |
| `ui/margins_battle.cpp:255` | gravely wounded |
| `ui/margins_battle.cpp:260` | The master of this place: once this one has fallen and the fight is over, the place is won. |
| `ui/margins_battle.cpp:265` | None in sight yet. |
| `ui/margins_battle.cpp:267` | To explore: in Single File (Q) or as a Group (G), click a door, a passage or stairs and the party walks there%s. |
| `ui/margins_battle.cpp:268` | ; Tab shows the level map (as far as the party has seen it), and a click on it walks the party there |
| `ui/margins_battle.cpp:277` | The fight is won. To go on, walk the party out of this place: click a way out (the edge of the street, a door, stairs). |
| `ui/menuview.cpp:106` | Arrows, Enter; Esc closes |
| `ui/menuview.cpp:106` | Click to choose; Esc or a click elsewhere closes |
| `ui/money.cpp:45` | %ld pfennig%s in all (1 florin = 20 groschen = 240 pfennigs) |
| `ui/nsdraw.cpp:26` |  against blows,  |
| `ui/nsdraw.cpp:27` |  against shots and throws; unusable with a two-handed weapon |
| `ui/nsdraw.cpp:31` | Missile Device |
| `ui/nsdraw.cpp:33` | , penetration  |
| `ui/nsdraw.cpp:34` | , time for a blow  |
| `ui/nsdraw.cpp:35` | draw and loose  |
| `ui/nsdraw.cpp:41` | Covers body and arms |
| `ui/nsdraw.cpp:41` | Covers the legs |
| `ui/nsdraw.cpp:63` | less well made |
| `ui/nsdraw.cpp:63` | as well made |
| `ui/nsdraw.cpp:68` |  wears nothing there. |
| `ui/nsdraw.cpp:72` | q): this is  |
| `ui/nsdraw.cpp:77` |  carries no shield. |
| `ui/nsdraw.cpp:78` | q): this is  |
| `ui/nsdraw.cpp:81` | Missile Device |
| `ui/nsdraw.cpp:86` |  Too heavy: it needs Strength  |
| `ui/nsdraw.cpp:90` |  In hand now:  |
| `ui/nsdraw.cpp:90` | , penetration  |
| `ui/nsdraw.cpp:91` |  No missile weapon equipped. |
| `ui/nsdraw.cpp:91` |  Nothing in hand now. |
| `ui/nsdraw.cpp:122` | Harder or better made than what  |
| `ui/nsdraw.cpp:123` |  nothing there. |
| `ui/nsdraw.cpp:124` | Everyone wears as good or better there. |
| `ui/nsdraw.cpp:134` | Better made than the shield of  |
| `ui/nsdraw.cpp:136` | Everyone's shield is as good or better. |
| `ui/nsdraw.cpp:139` | Missile Device |
| `ui/nsdraw.cpp:152` |  Too heavy for  |
| `ui/nsdraw.cpp:152` |  (it needs Strength  |
| `ui/nsdraw.cpp:171` | Many encounters let you use it. |
| `ui/nsdraw.cpp:171` | Some encounters let you use it. |
| `ui/nsdraw.cpp:171` | A few encounters let you use it. |
| `ui/nsdraw.cpp:171` | No encounter asks for it. |
| `ui/nsdraw.cpp:196` | No formula uses it. |
| `ui/nsdraw.cpp:197` |  (grade 2 only) |
| `ui/nsdraw.cpp:208` |  (every grade) |
| `ui/nsdraw.cpp:214` |  The party knows the formulae for  |
| `ui/nsdraw.cpp:219` | For climbing: over a town's walls at night, into a castle and the like, the best climber goes up first and lets it down for the others (without one, each climbs alone). One in the party is enough. |
| `ui/nsdraw.cpp:220` | For picking locks: shops at night, doors, chests, a cell. One in the party is enough. |
| `ui/nsdraw.cpp:221` | St. Cecilia, the patron of music, raises the party's standing in a town greatly for a member who carries an instrument. |
| `ui/nsdraw.cpp:223` | Nothing in the game asks for one (no card, no saint): it is worth only what it sells for. |
| `ui/nsdraw.cpp:226` | One of the Six Keys: the gate to the last chamber of the Baphomet's lair opens only for a party that carries all six. Keep it. |
| `ui/nsdraw.cpp:227` | A trophy: the boars' tusks, thought to be good-luck charms. Nothing in the game asks for them and no shop buys them. |
| `ui/nsdraw.cpp:228` | A trophy. Nothing in the game asks for it. |
| `ui/nsdraw.cpp:229` | A skeleton near a cemetery wants its bone back: give it this and it will talk with you. |
| `ui/nsdraw.cpp:230` | The dragon's gold cup, taken back from the thief: brandish it before the dragon and it will not breathe fire on its treasure. |
| `ui/nsdraw.cpp:232` | Wanted by whoever sent you to fetch it: bring it to them (the quests on the world map, M, say who and where). |
| `ui/nsdraw.cpp:244` |  for about a day (19 to 21 hours; a Darklands+ fix: in the original it never wears off). |
| `ui/nsdraw.cpp:244` |  permanently, adding up with the next dose (the original game never removes them). |
| `ui/nsdraw.cpp:246` | Thrown in a battle: a reeking cloud (up to 4 by 4 paces). Whoever it hits loses half their Agility, Perception and weapon skills and everyone else in the cloud a quarter, allies and enemies alike, while it lasts (longer at higher grades). |
| `ui/nsdraw.cpp:247` | Thrown: blinding powder. The target loses a third of their Agility, Perception and weapon skills and is blinded (cannot shoot, throw or take orders well); those beside them lose a quarter. Lasts longer at higher grades. |
| `ui/nsdraw.cpp:248` | Thrown: a cloud of black smoke. Shots through any cloud hit far less often (a fifth of the chance), and fighting from inside one costs 15 to hit. Lasts longer at higher grades. (The battle menu never offers it to the party: it is used on cards, and enemies throw it.) |
| `ui/nsdraw.cpp:249` | Thrown: itching powder. Weapon skills fall by five times the armor worn on the body and limbs: the heavier the armor, the worse the itch. Lasts longer at higher grades. |
| `ui/nsdraw.cpp:250` | Thrown: acid that eats the target's armor, lowering its quality by 4, 6 or 8 (grade 1, 2, 3) plus up to 8 more at random; armor eaten to nothing is destroyed, and where there is no armor (or damage is left over) it wounds the target. Also dissolves locks. |
| `ui/nsdraw.cpp:251` | Thrown: a poison cloud (up to 2 by 2 paces) whose wounds no armor stops, worse for weaker targets; stronger at higher grades. |
| `ui/nsdraw.cpp:252` | Thrown: a blinding flash. The target and every enemy of the thrower on that floor who faces it are blinded for a while (longer at higher grades); the thrower's allies are spared. |
| `ui/nsdraw.cpp:253` | Thrown: it bursts with great force: damage 4, 6 or 8 (grade 1, 2, 3) plus up to 8 more at random, penetration 6, 7 or 8 on the target and less around it; it also wears the armor down by a quarter of the damage. |
| `ui/nsdraw.cpp:254` | Thrown: burning pitch that no armor stops: damage 4, 6 or 8 (grade 1, 2, 3) plus up to 6 more at random, a quarter of that further off, minus a tenth of the armor's quality (and of Firewall's protection); it also wears the armor down. |
| `ui/nsdraw.cpp:255` | Thrown: a pool of sticky sludge (up to 4 by 4 paces) that slows whoever crosses it to 1/2, 3/8 or 1/4 of their pace (grade 1, 2, 3), for a long while. |
| `ui/nsdraw.cpp:256` | Used (P in the inventory) by a member: +3, +4 or +5 damage (grade 1, 2, 3) on every hit they land with any weapon except an impact weapon or a flail, as the manual says. Damage only, not the chance to hit; |
| `ui/nsdraw.cpp:257` | Used (P in the inventory) by a member: +3, +4 or +5 damage (grade 1, 2, 3) on every hit they land, with any weapon: the game ties it to the member, not a blade, so bows, guns, impact weapons and fists gain too (its label says non-impact weapons). Damage only, not the chance to hit; |
| `ui/nsdraw.cpp:258` | Used by a member: +3, +4 or +5 penetration (grade 1, 2, 3) on every hit they land with a blade or a polearm, as the manual says,  |
| `ui/nsdraw.cpp:258` | up to 15 in all |
| `ui/nsdraw.cpp:258` | past 15 the penetration starts again from 0, as in the original |
| `ui/nsdraw.cpp:259` | Used by a member: +3, +4 or +5 penetration (grade 1, 2, 3) on every hit they land, with any weapon (its label says blades and poles),  |
| `ui/nsdraw.cpp:259` | up to 15 in all |
| `ui/nsdraw.cpp:259` | past 15 the penetration starts again from 0, as in the original (a bow's 6 + 10 hits as 0) |
| `ui/nsdraw.cpp:260` | Used by a member: the weapon in hand counts as 14, 22 or 30 points higher in quality (grade 1, 2, 3; up to 99),  |
| `ui/nsdraw.cpp:260` | if it is an impact weapon, a flail or a handgun, as the manual says |
| `ui/nsdraw.cpp:260` | including bows, guns and fists |
| `ui/nsdraw.cpp:260` | , for about a day (19 to 21 hours). Each 10 points of quality add one step of damage. A second dose adds to the first. |
| `ui/nsdraw.cpp:261` | Used by a member, on every hit they land, with any weapon: grade 1 +14 penetration; grade 2 +1 damage and +6 penetration; grade 3 +1 damage and +14 penetration (the manual promises +14, +22 or +30 to the weapon's quality instead).  |
| `ui/nsdraw.cpp:261` | Penetration counts up to 15 in all |
| `ui/nsdraw.cpp:261` | As in the original, penetration past 15 starts again from 0, so grades 1 and 3 lower a bow's penetration |
| `ui/nsdraw.cpp:262` | Used by a member: +14, +22 or +30 (grade 1, 2, 3) to Bows, Thrown Weapons and Missile Devices (up to 99), for about a day (19 to 21 hours). A second dose adds to the first. |
| `ui/nsdraw.cpp:263` | Used by a member: their armor counts 20, 25 or 30 points better in quality and 1, 2 or 2 levels thicker (grade 1, 2, 3) against every blow and shot, as the manual says, for about a day (19 to 21 hours). A second dose while one is active keeps the stronger. |
| `ui/nsdraw.cpp:264` | Used by a member: +1, +2 or +3 (grade 1, 2, 3) to their armor against every hit, blow or shot, permanently: the game never removes it, and doses add up. |
| `ui/nsdraw.cpp:265` | Drinking it yields just one florin: it is meant for cards, where an encounter asks for it (to purify, or to make gold). |
| `ui/nsdraw.cpp:268` | 9-14, 15-21 or 22-29 |
| `ui/nsdraw.cpp:268` | 6-11, 8-13 or 10-15 |
| `ui/nsdraw.cpp:268` | 6-11, 8-13 or 10-15 |
| `ui/nsdraw.cpp:268` | 6-11, 8-15 or 10-17 |
| `ui/nsdraw.cpp:270` |  (grade 1, 2, 3) for about a day: the current value rises, past the maximum if need be; the maximum itself does not change.  |
| `ui/nsdraw.cpp:271` | On a wounded member it heals:  |
| `ui/nsdraw.cpp:271` | when it wears off, only the part still above the maximum is lost, so the healing stays. |
| `ui/nsdraw.cpp:271` | but when it wears off the game takes the whole amount back, healed or not (the Darklands+ fix "Blessings no longer cost attribute points for good", in F12 > Settings > Fixes, keeps the healed part). |
| `ui/nsdraw.cpp:272` | When it wears off, what it added is lost. |
| `ui/nsdraw.cpp:273` |  A second dose while the first is active adds only what goes beyond it. |
| `ui/nsdraw.cpp:275` | Drunk: permanently heals 50, 70 or 90% of the Strength lost to wounds (grade 1, 2, 3), and raises Endurance by 9-14, 15-21 or 22-29 for about 30 hours. |
| `ui/nsdraw.cpp:277` | Drunk: protection against fire for about a day (19 to 21 hours): Arabian Fire does 5, 7 or 9 points less (grade 1, 2, 3). A second dose keeps the stronger. |
| `ui/nsdraw.cpp:278` | Drunk: permanent protection against fire (the game never removes it): Arabian Fire does 5, 7 or 9 points less (grade 1, 2, 3). |
| `ui/nsdraw.cpp:278` |  As in the original, it also adds 16 to the armor permanently (a bug; the fix "No invulnerability from fire protection" in F12 > Settings > Fixes removes it). |
| `ui/nsdraw.cpp:294` |  when equipped |
| `ui/nsdraw.cpp:317` | No formula uses it. |
| `ui/nsdraw.cpp:347` | ; the other two grades don't use it |
| `ui/nsdraw.cpp:347` | ; the other grade doesn't use it |
| `ui/nsdraw.cpp:350` | Dots: the grades (1 to 3) whose formula uses it, gold where the party knows it. Point at a potion for more. |
| `ui/nsdraw.cpp:351` | Nobody in the party knows a formula with it. Dots: the grades (1 to 3) that use it; point at a potion for more. |
| `ui/nsdraw.cpp:421` | Load %d \xc2\xbb %d of %d |
| `ui/nsdraw.cpp:422` | Load %d of %d |
| `ui/nsdraw.cpp:475` | Spare equipment |
| `ui/nsdraw.cpp:475` | Tools and other goods |
| `ui/nsdraw.cpp:475` | Trophies and quest items |
| `ui/optpreview.cpp:42` | The gate is barred. You... |
| `ui/optpreview.cpp:44` | ...climb the wall |
| `ui/optpreview.cpp:45` | ...pray to a saint |
| `ui/optpreview.cpp:51` | St. Odilia: Jorg |
| `ui/optpreview.cpp:107` | Jorg hits the bandit |
| `ui/optpreview.cpp:107` | The bandit parries |
| `ui/optpreview.cpp:107` | Anna shoots: a wound |
| `ui/optpreview.cpp:107` | The bandit falls |
| `ui/optpreview.cpp:107` | Jorg: +1 Edged |
| `ui/optpreview.cpp:177` | Pointing at an enemy |
| `ui/optpreview.cpp:177` | The line of a shot |
| `ui/optpreview.cpp:177` | Skill gains, reloading |
| `ui/optpreview.cpp:177` | The battle log |
| `ui/optpreview.cpp:178` | One click to attack |
| `ui/optpreview.cpp:178` | Shoot or fight |
| `ui/optpreview.cpp:178` | Order buttons |
| `ui/optpreview.cpp:178` | Z: fast-forward |
| `ui/optpreview.cpp:179` | A member falls |
| `ui/optpreview.cpp:179` | Enemies come out |
| `ui/optpreview.cpp:180` | Tab: the level map; a click walks there |
| `ui/optpreview.cpp:181` | Chances, and who would try |
| `ui/ordericons.cpp:248` | walk towards |
| `ui/ordericons.cpp:248` | flee towards |
| `ui/ordericons.cpp:249` | travel as group |
| `ui/ordericons.cpp:249` | travel single file |
| `ui/ordericons.cpp:250` | dissolve lock |
| `ui/ordericons.cpp:251` | exit battlefield |
| `ui/ordericons.cpp:252` | cancel giving order |
| `ui/ornament.cpp:396` | Plaited copper |
| `ui/ornament.cpp:396` | Walnut and brass |
| `ui/ornament.cpp:396` | Gothic stone |
| `ui/ornament.cpp:518` | Pen flourishes |
| `ui/ornament.cpp:518` | Blind tooling |
| `ui/ornament.cpp:519` | Metal plaque |
| `ui/ornament.cpp:526` | Heals 8 to 15 Endurance |
| `ui/overlay.cpp:126` |   (replaced) |
| `ui/overlay.cpp:164` | A new encounter: %s (from a mod, not the original game) |
| `ui/overlay.cpp:176` | Darklands+ %s (build %s) |
| `ui/overlay.cpp:176` | Darklands+ build %s%s |
| `ui/overlay.cpp:210` | No controller detected |
| `ui/overlay.cpp:210` | Xbox controller detected |
| `ui/overlay.cpp:210` | PlayStation controller detected |
| `ui/overlay.cpp:210` | Nintendo controller detected |
| `ui/overlay.cpp:297` | MT-32 ACTIVE |
| `ui/overlay.cpp:322` | MIDI MESSAGE |
| `ui/overlay.cpp:323` | MIDI MESSAGE |
| `ui/overlay.cpp:381` | nobody knows |
| `ui/overlay.cpp:395` | nobody has one |
| `ui/overlay.cpp:404` | Saints who help here |
| `ui/overlay.cpp:404` | Items that help here |
| `ui/overlay.cpp:434` | nobody knows |
| `ui/overlay.cpp:434` | nobody has one |
| `ui/overlay.cpp:476` |  learned of  |
| `ui/overlay.cpp:602` |  when equipped |
| `ui/overlay.cpp:613` | Nobody in the party knows this saint |
| `ui/overlay.cpp:622` | %s - mystic number %d, %s |
| `ui/overlay.cpp:634` | %s of the three formulae for %s (the others: %s). |
| `ui/overlay.cpp:642` | Nobody in the party knows this formula. |
| `ui/overlay.cpp:657` | (carried: %d) |
| `ui/overlay.cpp:659` | Enough for %d batch%s. |
| `ui/overlay.cpp:659` | Not enough ingredients for a batch. |
| `ui/overlay.cpp:683` |   Take all of it |
| `ui/overlay.cpp:683` |   Share it out among the party |
| `ui/overlay.cpp:684` |   Sell the whole stack |
| `ui/overlay.cpp:718` | Making a character: name and gender first, then the family background, the attributes, and one profession after another, five years each, until you begin adventuring. Older characters know more, but age costs attributes. |
| `ui/overlay.cpp:721` | The family decides the first attributes and skills. The right side shows what the background under the pointer would give; EP (top right) are the points to spend next. |
| `ui/overlay.cpp:723` | Spend the EP on attributes: pick one, then + or -. Endurance and Strength are the hit points (and together set how much a character can carry); Agility counts in battle; Perception, Intelligence and Charisma serve many skills. |
| `ui/overlay.cpp:724` | Choose the profession for the next five years. The right side shows the one under the pointer: the EP it gives (top), the attributes after it, and for each skill gain:room, that is, what it adds at once and how many EP that skill can still take. Some professions open only after others, or need certain attributes. |
| `ui/overlay.cpp:725` | Spend the EP on skills: pick one, then + or -. A skill takes at most the room shown after the colon. Then choose the next profession, or begin adventuring. |
| `ui/overlay_text.cpp:176` | This option tests %s: %s %d%% chance of success. The original game shows this number while you hold Shift over an option; it rises with the skills and attributes tested. |
| `ui/overlay_text.cpp:177` | This option tests the whole party: %s %d%% chance of success. The original game shows this number while you hold Shift over an option; it rises with the skills and attributes tested. |
| `ui/overlay_text.cpp:211` | Edged Weapons |
| `ui/overlay_text.cpp:211` | Impact Weapons |
| `ui/overlay_text.cpp:211` | Flail Weapons |
| `ui/overlay_text.cpp:212` | Polearm Weapons |
| `ui/overlay_text.cpp:212` | Polearm Wpns |
| `ui/overlay_text.cpp:212` | Thrown Weapons |
| `ui/overlay_text.cpp:213` | Missile Devices |
| `ui/overlay_text.cpp:213` | Missile Dev. |
| `ui/overlay_text.cpp:214` | Speak Common |
| `ui/overlay_text.cpp:214` | Read & Write |
| `ui/overlay_text.cpp:216` | No Missile Weapon |
| `ui/overlay_text.cpp:216` | No Missile Wpn |
| `ui/overlay_text.cpp:217` | No Vitals Armor |
| `ui/overlay_text.cpp:217` | No Limb Armor |
| `ui/overlay_text.cpp:219` | Divine Favor |
| `ui/overlay_text.cpp:219` | Divine Favor |
| `ui/party.cpp:4` | Health and stamina. Wounds take it away; at 0 the character falls unconscious. Rest brings it back. |
| `ui/party.cpp:5` | Bodily strength. Weapons and armor need a minimum Strength (shown in the shops' tooltips), and Endurance and Strength together set how much a character can carry. Bad wounds take Strength too; at 0 the character dies. |
| `ui/party.cpp:7` | Speed and deftness, of the body and of the hands. |
| `ui/party.cpp:8` | Alertness: noticing things, people and dangers. |
| `ui/party.cpp:9` | Wits and learning. |
| `ui/party.cpp:10` | Presence: how people take to the character. |
| `ui/party.cpp:11` | The saints' goodwill. Calling on a saint spends it; praying at an inn, and time, bring it back. |
| `ui/party.cpp:15` | Mixing potions from the formulae a character knows: a better alchemist fails less often (the character sheet's Formulae tab shows each member's chance). |
| `ui/party.cpp:16` | Knowledge of the faith and its saints: learning of saints, dealings with the Church. |
| `ui/party.cpp:17` | Moral purity, valued by the saints and the Church. Sinful deeds lower it. |
| `ui/party.cpp:18` | Talking with ordinary people in their own tongue: asking, persuading, bluffing. |
| `ui/party.cpp:19` | The tongue of the Church and of scholars: priests, monks, universities. |
| `ui/party.cpp:20` | Literacy: letters, books and formulae. |
| `ui/party.cpp:21` | Tending wounds and sickness. |
| `ui/party.cpp:22` | Mechanical skill: locks, traps and devices. |
| `ui/party.cpp:23` | Moving and hiding unseen and unheard. |
| `ui/party.cpp:24` | The ways of the city streets and their underworld. |
| `ui/party.cpp:25` | Handling horses. |
| `ui/party.cpp:26` | Life in the wild: finding the way, hunting, reading tracks. |
| `ui/partypanel.cpp:236` | attacks (Std Attack, a standard attack) |
| `ui/partypanel.cpp:237` | attacks with Vulnerable: aims at the gaps in the armor (1 to 4 more penetration), slower and a little less likely to land |
| `ui/partypanel.cpp:238` | is berserk: attacks without thought of defense |
| `ui/partypanel.cpp:240` | picks the lock of the door given |
| `ui/partypanel.cpp:241` | picks the lock of the chest given |
| `ui/partypanel.cpp:242` | parries: mostly defends, hard to hit, striking back slower and less surely |
| `ui/partypanel.cpp:243` | shoots at the target given |
| `ui/partypanel.cpp:244` | throws a potion at the target given |
| `ui/partypanel.cpp:245` | walks to the spot given |
| `ui/partypanel.cpp:246` | flees to the spot given: does not stop to fight or strike back |
| `ui/partypanel.cpp:247` | takes the stairs given |
| `ui/partypanel.cpp:247` | opens or shuts the door given |
| `ui/partypanel.cpp:248` | dissolves the lock given |
| `ui/partypanel.cpp:249` | opens the chest given |
| `ui/partypanel.cpp:250` | leaves the battlefield by the chosen exit |
| `ui/partypanel.cpp:251` | halts and stands where they are |
| `ui/partypanel.cpp:254` | has no clear line of fire to the target: steps aside or nearer until there is one |
| `ui/partypanel.cpp:255` | has no clear line of fire to the target and waits for one (or give another order) |
| `ui/partypanel.cpp:256` | has nothing left to shoot or throw: give another order |
| `ui/partypanel.cpp:257` | waits for an order: click them here or on the battlefield, then an enemy, a spot or a choice from the Orders menu |
| `ui/partypanel.cpp:437` | %s carries %s: %d against %d (Endurance + Strength; only equipped items count). %s %s. |
| `ui/partypanel.cpp:438` | far too much |
| `ui/partypanel.cpp:438` | a heavy load |
| `ui/partypanel.cpp:459` | In effect on %s:%s |
| `ui/partypanel.cpp:473` | %s opens with the missile weapon: a click on an enemy makes %s shoot while the line is clear, then fight hand to hand once the enemies close in. Click here: melee from the start. |
| `ui/partypanel.cpp:474` | %s opens with the melee weapon: a click on an enemy makes %s close in and fight hand to hand. Click here: shoot first, then fight hand to hand once the enemies close in. |
| `ui/prayview.cpp:37` |  calls on the saint for the whole party: |
| `ui/prayview.cpp:37` |  calls on the saint: |
| `ui/prayview.cpp:39` | Saint's day:  |
| `ui/prayview.cpp:45` | Differs from the description:  |
| `ui/prayview.cpp:53` | Divine Favor to spend |
| `ui/prayview.cpp:87` | %s has %d; %d will be left. Past the green mark the chance rises no more. |
| `ui/prayview.cpp:87` | %s has %d; %d will be left. |
| `ui/prayview.cpp:90` | Chance the saint hears: %d%% |
| `ui/prayview.cpp:93` | More Divine Favor gives a better chance (up to 99%); it is spent either way. |
| `ui/prayview.cpp:113` | %s's Virtue (%d) is below the %d this saint asks: more Divine Favor will not help. |
| `ui/prayview.cpp:114` | This saint asks at least %d Divine Favor; %s has only %d. |
| `ui/prayview.cpp:115` | This saint asks at least %d Divine Favor: spend more. |
| `ui/prayview.cpp:116` | No chance as it is: spend more Divine Favor (or the saint does not help here). |
| `ui/professions.cpp:340` | Only for a man |
| `ui/professions.cpp:345` | Not with this character's past |
| `ui/professions.cpp:345` | , and not with this past |
| `ui/professions.cpp:350` | ; also short of  |
| `ui/professions.cpp:359` | Speak Common |
| `ui/professions.cpp:359` | Read & Write |
| `ui/pstone.cpp:211` | The philosophers' egg |
| `ui/pstone.cpp:211` | The stone itself |
| `ui/pstone.cpp:211` | The squared circle |
| `ui/pstone.cpp:214` | black (nigredo), the raven |
| `ui/pstone.cpp:214` | white (albedo), the swan |
| `ui/pstone.cpp:214` | yellow (citrinitas), the peacock's colors |
| `ui/pstone.cpp:214` | red (rubedo), the phoenix |
| `ui/pstone.cpp:233` | 5 (1-7: black) |
| `ui/pstone.cpp:233` | 12 (8-14: white) |
| `ui/pstone.cpp:233` | 19 (15-21: yellow) |
| `ui/pstone.cpp:233` | 29 (22-29: red) |
| `ui/questicons.cpp:17` | Robber knight |
| `ui/questicons.cpp:17` | Reward waiting |
| `ui/questicons.cpp:17` | A treasure to fetch |
| `ui/questicons.cpp:17` | Papers to get in a town |
| `ui/questicons.cpp:18` | Unwelcome in a town |
| `ui/questicons.cpp:18` | Wanted in a town |
| `ui/questicons.cpp:18` | City politics |
| `ui/quests.cpp:56` | no time limit |
| `ui/quests.cpp:94` | the pagan altar north-west of Hall |
| `ui/quests.cpp:114` | the lake near Freiberg |
| `ui/quests.cpp:156` | a foreign trader |
| `ui/quests.cpp:156` | a pharmacist |
| `ui/quests.cpp:157` | the villagers |
| `ui/quests.cpp:158` | the city's lord |
| `ui/quests.cpp:194` | the leading merchant |
| `ui/quests.cpp:194` | the Hanseatic League |
| `ui/quests.cpp:296` | no time limit |
| `ui/quests.cpp:298` | Robber knight @  |
| `ui/quests.cpp:298` | Kill the robber knight  |
| `ui/quests.cpp:299` |  A village pays no bounty: its reward is the village's good opinion of the party when he falls. |
| `ui/quests.cpp:304` | Bounty to collect @  |
| `ui/quests.cpp:304` | The robber knight is dead: collect the bounty from  |
| `ui/quests.cpp:304` |  (at the cathedral, by day: at night it holds no audience) |
| `ui/quests.cpp:308` | The offices of the merchants and bankers are above  |
| `ui/quests.cpp:308` | 's marketplace; at night the party can sneak up to them. |
| `ui/quests.cpp:316` | Dragon near  |
| `ui/quests.cpp:316` | A dragon terrorizes the land around  |
| `ui/quests.cpp:318` | Trouble at the mine:  |
| `ui/quests.cpp:320` | The Wild Hunt |
| `ui/quests.cpp:320` | The Wild Hunt pursues the party. |
| `ui/quests.cpp:322` | Unwelcome @  |
| `ui/quests.cpp:322` | Not welcome in  |
| `ui/quests.cpp:322` | : the town counts as hostile to the party (its gates and people turn it away). |
| `ui/quests.cpp:324` | Wanted by the watch in  |
| `ui/quests.cpp:324` | : guards who recognize the party try to arrest it, and talking or bribing your way out is harder. |
| `ui/quests.cpp:327` | Witches' Sabbat at  |
| `ui/quests.cpp:328` | Witches gather at  |
| `ui/quests.cpp:328` |  (the night is not known). |
| `ui/quests.cpp:329` | Witches gather on the night of  |
| `ui/quests.cpp:329` |  (the place is not known). |
| `ui/quests.cpp:333` | Rebels @ You have joined the rebels of . Post their handbills, win the guilds over (ask about their leaders and politics, meet their masters), and when the cause is strong ask your leader for a war council: it sets the day of the march. You reach the leader through the agitators' guild or the side streets (other locations you remember). |
| `ui/quests.cpp:337` | For the government @ You serve the government of  against the rebels. Post its handbills, win the guilds over, and when the time is right ask your patron for a war council: it sets the day of the arrests. Your patron's house is in the side streets (other locations you remember). |
| `ui/quests.cpp:346` | Your side marches on the  |
| `ui/quests.cpp:347` |  that day: if you are at the leader's house, the rally starts there; if you are in the streets, the march finds you. |
| `ui/quests.cpp:348` | The city guard arrests the rebel leaders at  |
| `ui/quests.cpp:349` |  that day: if you are at your patron's house, the arrest starts there; if you are in the streets, the escort finds you. |
| `ui/quests.cpp:353` | Sabbat now @  |
| `ui/quests.cpp:353` | A witches' Sabbat is being held now at  |
| `ui/quests.cpp:455` |  (The same giver asked twice.) |
| `ui/quests.cpp:488` | %s, under a day away |
| `ui/quests.cpp:489` | %s, about %d day%s away |
| `ui/quests.cpp:534` |  on foot as the crow flies (roads and rivers make it longer) |
| `ui/quests.cpp:546` | Start or load a game first. |
| `ui/quests.cpp:553` | Rewards waiting |
| `ui/quests.cpp:553` | Unwelcome or wanted |
| `ui/quests.cpp:554` | In order of: |
| `ui/quests.cpp:559` |   (click a quest to open it) |
| `ui/quests.cpp:582` | Travel there |
| `ui/quests.cpp:590` | No quests at the moment. Robber-knight bounties, errands and deliveries the party takes on are listed here with their deadlines. |
| `ui/quests.cpp:592` | Show secrets (Sabbat dates and places, even before the party learns them) |
| `ui/rosterview.cpp:33` |  Add to the Party |
| `ui/rosterview.cpp:67` | Who travels on |
| `ui/rosterview.cpp:68` | Up to four travel together. Those who stay behind are kept here and can join again; new characters can join too. |
| `ui/rosterview.cpp:69` | Up to four travel together. Choose who goes, make new characters, then begin. |
| `ui/rosterview.cpp:83` | Choose someone below, then  |
| `ui/rosterview.cpp:83` | Add to the party |
| `ui/rosterview.cpp:83` | Make a character first |
| `ui/rosterview.cpp:111` | brewing potions from the formulae the party knows |
| `ui/rosterview.cpp:112` | the saints: learning them and calling on them; churches and the clergy |
| `ui/rosterview.cpp:112` | standing firm against evil; the saints' favor; some cards |
| `ui/rosterview.cpp:113` | talking: bargaining, persuading, most people on the cards |
| `ui/rosterview.cpp:113` | talking with the learned, the clergy and the nobility |
| `ui/rosterview.cpp:114` | reading letters, books, papers and inscriptions |
| `ui/rosterview.cpp:114` | tending wounds after a fight |
| `ui/rosterview.cpp:114` | locks, traps and devices |
| `ui/rosterview.cpp:115` | sneaking, hiding, getting in unseen |
| `ui/rosterview.cpp:115` | the towns' underside: thieves, rumors, the streets at night |
| `ui/rosterview.cpp:116` | riding: faster travel, outriding pursuers on the cards |
| `ui/rosterview.cpp:116` | the wilds: finding the way, hunting, tracking, keeping out of danger |
| `ui/rosterview.cpp:118` | The party's best skills |
| `ui/rosterview.cpp:119` | Each skill's best level in the party. Point at one to see what it is for. |
| `ui/rosterview.cpp:119` | Add characters to the party to see its skills together. |
| `ui/rosterview.cpp:142` | Characters (%d) |
| `ui/rosterview.cpp:144` | double-click: add / remove |
| `ui/rosterview.cpp:166` | in the party |
| `ui/rosterview.cpp:178` | No characters are kept yet. Make one with Create a Character. |
| `ui/rosterview.cpp:189` |   -  in the party |
| `ui/rosterview.cpp:216` | Knows %d saint%s and %d formula%s; carries %d item%s. |
| `ui/rosterview.cpp:223` | Leave at the inn |
| `ui/rosterview.cpp:223` | Remove from the party |
| `ui/rosterview.cpp:225` | Add to the party |
| `ui/rosterview.cpp:227` | Click again to delete |
| `ui/rosterview.cpp:227` | Delete character |
| `ui/rosterview.cpp:248` | Add to the party to set sprite, heraldry and colors. |
| `ui/rosterview.cpp:249` | Choose a character on the left. |
| `ui/rosterview.cpp:252` | New character |
| `ui/rosterview.cpp:253` | Begin the adventure |
| `ui/rosterview.cpp:256` | %d of 4 in the party |
| `ui/rules.cpp:44` | missile devices |
| `ui/rules.cpp:107` | rules.ini line  |
| `ui/rules.cpp:107` | : unknown section  |
| `ui/rules.cpp:129` | rules.ini line  |
| `ui/rules.cpp:129` | : unknown field  |
| `ui/rules.cpp:185` | # the original game's numbers stay for any weapon not named here. # skill: edged, impact, flails, polearms, thrown, bows, missile devices. time: for a blow (lower is quicker). # strength = min-max: below min each point costs 3 melee rating (1 aim for a missile weapon); above max, 2 melee rating more and harder blows.   |
| `ui/rules.cpp:205` | The battle numbers of every weapon, as the game uses them now%s. |
| `ui/rules.cpp:205` |  (changed by a mod) |
| `ui/rules.cpp:206` | Missile dev. |
| `ui/rules.cpp:209` | Skill needed |
| `ui/rules.cpp:246` | Time: how long a blow takes (lower is quicker); where it is blank, Reload sets the pace. Hand to hand, a fighter's melee rating counts both in attack and in defense; each point of skill below what a weapon needs costs 2 of it, each point of Strength below the weapon's range 3 (and weaker blows), each point above the range adds 2 (and harder blows). With a missile weapon, the aim: 2 per point of skill short, 1 per point of Strength short. |
| `ui/rules.cpp:251` | Write these numbers into a mod folder (rules.ini) |
| `ui/rules.cpp:254` | Could not make the mod folder. |
| `ui/rules.cpp:255` | . Edit it, then tick the mod in F12 > Mods. |
| `ui/rules.cpp:256` | Could not write  |
| `ui/saints.cpp:48` | stays with the party |
| `ui/saints.cpp:71` | %+d armor (from a saint) |
| `ui/saints.cpp:71` | worn armor %+d quality (from a saint) |
| `ui/saints.cpp:72` | metal armor %+d quality (from a saint) |
| `ui/saints.cpp:72` | leather and cloth armor %+d quality (from a saint) |
| `ui/saints.cpp:73` | weapon %+d quality (from a saint) |
| `ui/saints.cpp:73` | edged weapon %+d quality (from a saint) |
| `ui/saints.cpp:74` | blades, maces, polearms, crossbows and guns %+d quality (from a saint) |
| `ui/saints.cpp:75` | fire protection %d (from a saint) |
| `ui/saints.cpp:75` | %+d armor against missiles (from a saint) |
| `ui/saints.cpp:76` | the party travels %d%% faster (from a saint) |
| `ui/saints.cpp:76` | the party travels %d%% faster in woods (from a saint) |
| `ui/saints.cpp:77` | no death from wounds (from a saint) |
| `ui/saints.cpp:80` | the party travels %s%s (from a saint) |
| `ui/saints.cpp:80` | twice as fast |
| `ui/saints.cpp:80` | half again as fast |
| `ui/saints.cpp:87` | %s: %+d damage |
| `ui/saints.cpp:88` | %s: %+d penetration |
| `ui/saints.cpp:90` | %s: armor %+d thickness, %+d quality |
| `ui/saints.cpp:91` | %s: fire protection %d |
| `ui/saints.cpp:92` | %s: weapon quality %+d |
| `ui/saints.cpp:93` | %s: %+d penetration |
| `ui/saints.cpp:94` | %s: %+d damage, %+d penetration |
| `ui/saints.cpp:168` | Provides ... |
| `ui/saints.cpp:168` | Also heals ... |
| `ui/saints.cpp:184` | ..., Agnes grants |
| `ui/saints.cpp:222` | What it does: %s |
| `ui/saints.cpp:228` | Differs from the description: %s |
| `ui/saints.cpp:238` | Many encounters offer this saint as a choice, and there it can do more than the help above. |
| `ui/saints.cpp:239` | Some encounters offer this saint as a choice, and there it can do more than the help above. |
| `ui/saints.cpp:240` | A few encounters offer this saint as a choice, and there it can do more than the help above. |
| `ui/saints.cpp:241` | No encounter calls for this saint: its help above is all it does. |
| `ui/saves.cpp:41` | yesterday %H:%M |
| `ui/saves.cpp:53` | Quick save, quick load and autosaves |
| `ui/saves.cpp:57` | Ctrl+S saves at once (no comment to type); F9 or Ctrl+L loads the last quick save. |
| `ui/saves.cpp:70` | as a quick save, when the game next allows saving (not in shops or during conversations). |
| `ui/saves.cpp:73` | Save and Load: the Game menu (right-click), or Alt+S and Alt+L. Quick save and Quick load are also in the menu bar's Darklands+ menu. |
| `ui/saves.cpp:81` | No saved games yet. |
| `ui/saves.cpp:82` | %d saved game%s, newest first |
| `ui/saves.cpp:84` | Loading from here works once a game is running; before that, use Load on the title screen. |
| `ui/saves.cpp:88` | Where and when |
| `ui/saves.cpp:100` |   (in battle) |
| `ui/savesview.cpp:30` | %lld min ago |
| `ui/savesview.cpp:33` | %lld days ago |
| `ui/savesview.cpp:69` | Load a saved game |
| `ui/savesview.cpp:69` | Save the game |
| `ui/savesview.cpp:75` | Close options |
| `ui/savesview.cpp:75` | Saving options |
| `ui/savesview.cpp:80` | Saving options |
| `ui/savesview.cpp:86` | Save name (up to 22 characters): |
| `ui/savesview.cpp:97` | Existing saves |
| `ui/savesview.cpp:98` | click it again not to replace it |
| `ui/savesview.cpp:98` | click one to replace it with this save |
| `ui/savesview.cpp:101` | Double-click a save to load it. |
| `ui/savesview.cpp:122` | No saved games yet. |
| `ui/savesview.cpp:153` | in a battle,  |
| `ui/savesview.cpp:154` | the last loaded or saved,  |
| `ui/savesview.cpp:154` | the last used,  |
| `ui/savesview.cpp:156` | will be replaced by this save |
| `ui/savesview.cpp:157` | click again to delete it for good |
| `ui/screenpics.cpp:24` | The character sheet |
| `ui/screenpics.cpp:131` | The world map (M) |
| `ui/screenpics.cpp:173` | The overland view |
| `ui/settings.cpp:74` | Use replacement art from the "art" folder |
| `ui/settings.cpp:80` | No replacement pictures yet (%s%sart). |
| `ui/settings.cpp:82` | %d replacement pictures found. |
| `ui/settings.cpp:84` | Export the original pictures |
| `ui/settings.cpp:88` | For artists: a file art%sNAME.png (any size; transparency where the picture has color 0) replaces the game's picture NAME wherever it is shown, at the window's resolution. Export writes every original picture to art%soriginal as a template, in its own colors. Whatever the game draws over a picture (text, the mouse pointer, other pictures) stays on top. |
| `ui/settings.cpp:92` | More crests: art%sSHIELDP.png, SHIELDQ.png ... up to SHIELDZ.png add crests after the game's fifteen (Heraldry, when making a party). A save with a member using one of these crests loads only in Darklands+. |
| `ui/settings.cpp:106` | A mod is a folder in %smods with files laid out like the game folder: while the mod is ticked, the game reads the mod's files in place of the originals (the game folder is never changed). |
| `ui/settings.cpp:108` | No mods found yet. |
| `ui/settings.cpp:109` | Where two mods change the same file, the one lower in the list wins. Drag a mod by its handle (or use its arrows) to reorder. |
| `ui/settings.cpp:163` | New encounters from mods: %d%s |
| `ui/settings.cpp:164` | (%d file%s with errors left out: see darklands_port.log) |
| `ui/settings.cpp:169` | At 100%%, each new encounter is as likely as any one of the original encounters for that terrain (or more, if its file gives it a weight). |
| `ui/settings.cpp:175` | Look for mods again |
| `ui/settings.cpp:180` | Make a new mod folder |
| `ui/settings.cpp:182` | Made %s (with a mod.txt that explains what goes where). |
| `ui/settings.cpp:182` | %s is there already. |
| `ui/settings.cpp:182` | Could not make %s. |
| `ui/settings.cpp:184` | Export the card texts |
| `ui/settings.cpp:186` | %d card files written to %s as text. Copy the ones you change into your mod's MSGFILES folder. |
| `ui/settings.cpp:187` | Could not read MSGFILES. |
| `ui/settings.cpp:189` | every town and encounter text, editable, with its options |
| `ui/settings.cpp:191` | Write every text shown to a file (for a translation: mods |
| `ui/settings.cpp:192` | A mod's text.txt replaces the texts built into the game's menus, one per line, e.g. Purchase an item => Kaufen (needs New text: F12 > Settings > Text). |
| `ui/settings.cpp:194` | Mods are applied in the order of the list above: where two change the same file, the lower one wins. Single files inside MSGFILES and the .CAT catalogs can be replaced by a folder of that name holding just those files. Card texts and pictures change at once; the item list, saints, cities and the like change the next time the game starts. |
| `ui/settings.cpp:199` | Replacement art (for artists) |
| `ui/settings.cpp:203` | Built-in mods: easier play |
| `ui/settings.cpp:204` | Off while playing the original game (Full Original): pick another preset to tick these. |
| `ui/settings.cpp:205` | For the story more than the struggle. Each is its own switch; no preset turns them on, and Full Original and Purist turn them off. Some take effect only when the next new game starts (their note says so). |
| `ui/settings.cpp:325` | Pictures of the new screens appear here as you meet them in your game: a card, a shop, the character sheet, the inn, alchemy. |
| `ui/settings.cpp:332` | A picture of the world map appears here once you have opened it (M) in your game. |
| `ui/settings.cpp:339` | A picture of the overland view, with the names and quest markers on it, appears here once the party has been out in the land. |
| `ui/settings.cpp:379` |  (recommended) |
| `ui/settings.cpp:387` | your own choices:  |
| `ui/settings.cpp:400` |  and %d more |
| `ui/settings.cpp:407` | your own choices from before you picked a preset, kept: click to restore them. |
| `ui/settings.cpp:416` | Ironman, for your next new game |
| `ui/settings.cpp:416` | one save, kept by the game itself: saved over on every card, on arriving at a place, before each battle and when you quit.  |
| `ui/settings.cpp:416` | No loading back while that game runs, and when the party's journey ends, its save is deleted. Any preset. |
| `ui/settings.cpp:419` | The game under way is an Ironman game. |
| `ui/settings.cpp:423` | Saved games stay compatible with the original game both ways, whatever the preset. |
| `ui/settings.cpp:425` | Darklands+ saves in the original game's format: its saves load in the original (on DOS too), and the original's saves load here; what Darklands+ records on top (what the party has learned of the witches' Sabbats, items marked Keep, sprites and colors) goes into separate files of its own. Switching presets or single switches at any time is safe. |
| `ui/settings.cpp:429` | One exception: |
| `ui/settings.cpp:431` | a save with a member using an enemy sprite or an added crest (Darklands+ offers both) loads only in Darklands+. |
| `ui/settings.cpp:437` | The band around the windows |
| `ui/settings.cpp:439` | The clock while hours pass |
| `ui/settings.cpp:444` | Frames and clock |
| `ui/settings.cpp:507` | Everything Darklands+ adds is off, and its switches are hidden: pick another preset to change single switches. |
| `ui/settings.cpp:522` | The original game's bugs put right (%d of %d on). Each is its own switch, since what counts as a bug (and how to mend it) is open to argument; they are sorted from the plainly broken to the arguable. |
| `ui/settings.cpp:557` | Reconstructed Content |
| `ui/settings.cpp:560` | What MicroProse wrote or coded for Darklands and never switched on, put back by Darklands+. Nobody played it in 1992: expect rough edges. Every switch here is off unless you tick it, and every preset turns them off again. |
| `ui/settings.cpp:562` | What each one was in the game's files, and what Darklands+ had to add or put right, is in docs/cut_content.md (and docs/cut_politics.md). |
| `ui/settings.cpp:564` | Off while playing the original game (Full Original): pick another preset to tick these. |
| `ui/settings.cpp:569` | Written, but never finished |
| `ui/settings.cpp:569` | Finished, but never switched on |
| `ui/settings.cpp:570` | The texts or pictures are in the game's files, but the game never shows them: Darklands+ supplies the missing pieces. |
| `ui/settings.cpp:571` | Written and coded by MicroProse, only never turned on (or, for Black Cloud, left out of a menu): Darklands+ turns it on. |
| `ui/settings_look.cpp:91` | A potion's grade, on its flask |
| `ui/settings_look.cpp:93` | Chevrons on a badge |
| `ui/settings_look.cpp:93` | Dots on a strip |
| `ui/settings_look.cpp:125` | The gate of the town stands open. |
| `ui/settings_look.cpp:127` | The original lettering |
| `ui/settings_look.cpp:151` | (the game's fonts were not found) |
| `ui/settings_look.cpp:164` | New text: the same words drawn sharp at any window size, outlined over pictures and darkened where faint; the character sheet's abbreviations spelled out (wPol: Polearm Wpns), names in full where the game cut them short, armor named by where it is worn (Chainmail (body) for V:Chainmail), and icons beside the skills. |
| `ui/settings_look.cpp:179` | The game's own small blackletter, as in 1992: true to the original, but hard to read on a large screen. |
| `ui/settings_look.cpp:207` | Pen flourishes |
| `ui/settings_look.cpp:207` | Stamped leather |
| `ui/settings_look.cpp:208` | Metal plaques |
| `ui/settings_look.cpp:208` | Flat, gold-edged |
| `ui/settings_look.cpp:208` | Rounded tabs |
| `ui/settings_look.cpp:209` | Frames (tooltips, the card text, the new screens' pages) |
| `ui/settings_look.cpp:231` | The band around the windows, after the original's own: the cards' plaited copper, the party sheet's walnut and brass, the illuminated capitals' vermilion and gold, the stone niche around the figure on the character sheet. With any of them, the lines and corners are gilt and the frames and buttons are as chosen here. |
| `ui/settings_look.cpp:264` | Shown while hours pass (at the inn, waiting, traveling on a card). The tower dial follows the great clocks of about 1400 (Wells, Prague); the copper astrolabe is the game's own clock, drawn anew so that it stays sharp. |
| `ui/settings_setup.cpp:72` | Connected: %s (%s style) |
| `ui/settings_setup.cpp:73` | No controller connected now: plug one in at any time. The buttons below can be set up beforehand. |
| `ui/settings_setup.cpp:76` | Show the buttons as: |
| `ui/settings_setup.cpp:78` | Right-click (the menus) |
| `ui/settings_setup.cpp:78` | Tab (the level map) |
| `ui/settings_setup.cpp:78` | Space (pause) |
| `ui/settings_setup.cpp:78` | F12 (the Darklands+ window) |
| `ui/settings_setup.cpp:78` | M (the world map) |
| `ui/settings_setup.cpp:78` | F11 (keys and clicks) |
| `ui/settings_setup.cpp:99` | the pointer (the left trigger held: slowly) |
| `ui/settings_setup.cpp:99` | scrolls lists; pressed: Z (fast-forward) |
| `ui/settings_setup.cpp:100` | the arrow keys |
| `ui/settings_setup.cpp:100` | held: Ctrl (Ctrl+click in the overland view travels there) |
| `ui/settings_setup.cpp:105` | Back to the usual buttons |
| `ui/settings_setup.cpp:107` | Match the device in use |
| `ui/settings_setup.cpp:107` | Keys and mouse always |
| `ui/settings_setup.cpp:107` | PlayStation always |
| `ui/settings_setup.cpp:107` | Nintendo always |
| `ui/settings_setup.cpp:111` | While a controller is in use, the hints and on-screen buttons show its buttons. The "always" choices are for a Steam Deck played with its trackpads, which count as a mouse. |
| `ui/settings_setup.cpp:119` | Sharp pixels |
| `ui/settings_setup.cpp:119` | Rounded edges |
| `ui/settings_setup.cpp:135` | (no picture yet) |
| `ui/settings_setup.cpp:140` | Every pixel a crisp square, as the game drew it. |
| `ui/settings_setup.cpp:141` | The stair-steps of diagonal edges rounded off, without blurring. On a computer without a proper graphics card, the game slows down with this. |
| `ui/settings_setup.cpp:142` | Blurred a little, as an old monitor would. |
| `ui/settings_setup.cpp:152` | text size in the margins, tooltips and place names |
| `ui/settings_setup.cpp:161` | automatic (the system's scale) |
| `ui/settings_setup.cpp:170` | Full screen (Alt+Enter) |
| `ui/settings_setup.cpp:175` | None: straight to the start screen |
| `ui/settings_setup.cpp:175` | Only the last scene: the lady of the Darklands and her welcome |
| `ui/settings_setup.cpp:175` | The whole intro |
| `ui/settings_setup.cpp:178` | Shown each time the game starts (the very first start plays it whole). A key or a click skips ahead. |
| `ui/settings_setup.cpp:198` | Preview: Battle Sounds |
| `ui/settings_setup.cpp:199` | Preview: Music |
| `ui/settings_setup.cpp:204` | Preview: Music |
| `ui/settings_setup.cpp:213` | Preview: Battle Sounds |
| `ui/settings_setup.cpp:219` | A preview needs a game under way, outside a battle. |
| `ui/settings_setup.cpp:231` | Roland MT-32 |
| `ui/settings_setup.cpp:231` | Roland's synthesizer of 1987, the one Darklands was written for. Music and battle sounds alike are notes played on it: the music as composed, the blows as near to steel as a synthesizer gets. |
| `ui/settings_setup.cpp:239` | MT-32 ROMs not found (click to close) |
| `ui/settings_setup.cpp:239` | MT-32 ROMs not found (click for how to get it working) |
| `ui/settings_setup.cpp:239` | MT-32 ROMs not found (click for how to get it working) |
| `ui/settings_setup.cpp:242` | MT-32 details |
| `ui/settings_setup.cpp:246` | The MT-32 was a Roland synthesizer of 1987; Darklands' music and battle sounds were written for it (both are notes played on it), and every other choice here is a lesser version. Darklands+ has it built in (Munt's imitation of it) and only needs two files copied from a real one's memory (its ROM files), which belong to Roland and can't be included. If you have used Munt or DOSBox with an MT-32 you already have them. Until they are found, the Sound Blaster plays. |
| `ui/settings_setup.cpp:249` | Where to find them... |
| `ui/settings_setup.cpp:256` | Look again##mt32 |
| `ui/settings_setup.cpp:257` | Darklands+ finds them by their contents, whatever their names, in that folder and in: |
| `ui/settings_setup.cpp:266` | midiA synthesizer outside Darklands+ (MIDI)For a real MT-32 connected to the computer, or Munt (an MT-32 imitation) installed on its own. An ordinary synthesizer, such as the one built into Windows, plays the wrong instruments.midi |
| `ui/settings_setup.cpp:271` | No MIDI outputs (connections to a synthesizer) on this computer. |
| `ui/settings_setup.cpp:274` | the system's default |
| `ui/settings_setup.cpp:287` | For history's sake |
| `ui/settings_setup.cpp:292` | A change is heard from the next battle, or the next start. |
| `ui/settings_setup.cpp:305` | Every piece of the game's music, played through the device chosen above. Click a piece to hear it. |
| `ui/settings_setup.cpp:306` | Every piece of the game's music, played through the device chosen above: it needs a game under way, outside a battle. |
| `ui/settings_setup.cpp:331` | finding its length (silent, a few seconds) |
| `ui/settings_setup.cpp:332` | , then again |
| `ui/settings_setup.cpp:333` | no piece playing |
| `ui/settings_setup.cpp:342` | (a piece without a known place) |
| `ui/settings_setup.cpp:359` | Custom sound packs |
| `ui/settings_setup.cpp:360` | Recordings that replace the synthesizer's music or battle sounds. A battle sounds pack comes with Darklands+ as a mod (F12 > Mods); no music pack does. The previews above play them when ticked. |
| `ui/settings_setup.cpp:363` | Music from a music pack (recordings in a "music" folder) |
| `ui/settings_setup.cpp:369` | No music pack found: a mod's music folder, or %s%smusic, with a recording for each piece named by its number (10.ogg, "11 market.ogg"). |
| `ui/settings_setup.cpp:371` | The game's music now: piece %d%s%s%s, %s (silent while Settings is open). |
| `ui/settings_setup.cpp:371` | from the pack |
| `ui/settings_setup.cpp:371` | by the synthesizer |
| `ui/settings_setup.cpp:373` | A recording replaces a piece when its file name starts with the number (up to four for one number, played in turn: "27 a.ogg", "27 b.ogg" ...). Pieces that loop in the game loop in the pack; while a piece of the pack is on, the synthesizer is silent. |
| `ui/settings_setup.cpp:379` |   %s and %d more, in turn |
| `ui/settings_setup.cpp:381` | Look again##music |
| `ui/settings_setup.cpp:387` | Ordinary hits make a sound |
| `ui/settings_setup.cpp:388` | Battle sounds from a sound pack (the "sounds" folder) |
| `ui/settings_setup.cpp:390` | %d sound%s in %s%ssounds; the game's own sounds for the rest. |
| `ui/settings_setup.cpp:391` | Recorded blows, parries and cries in place of the synthesizer's: a pack in %s%ssounds, or the "Battle sounds" mod (F12 > Mods). |
| `ui/settings_setup.cpp:396` | Volume of the pack |
| `ui/settings_setup.cpp:398` | Look again##sfx |
| `ui/settings_setup.cpp:398` | %d sound%s found. |
| `ui/settings_setup.cpp:402` | File names for a sound pack |
| `ui/settings_setup.cpp:403` | %s.wav or .ogg: %s |
| `ui/settings_setup.cpp:404` | Several versions of one sound (parry1.wav, parry2.wav ...) are played in turn at random. |
| `ui/sheetview.cpp:159` | Light load (up to half of Endurance + Strength): no hindrance at all. Only what is equipped counts (weapons, armor, shield); items merely carried weigh nothing. |
| `ui/sheetview.cpp:160` | Normal load (up to Endurance + Strength): blows hand to hand come about a fifth slower, and past 5/8 of it the Agility used to dodge falls to two thirds. |
| `ui/sheetview.cpp:161` | Heavy load (up to half as much again): the Agility used to dodge falls to two thirds, and to a third past 1 1/4 times Endurance + Strength. Blows are no slower than with a normal load (an oversight in the original game). |
| `ui/sheetview.cpp:162` | Overloaded (more than 1 1/2 times Endurance + Strength): blows hand to hand come about two thirds slower, and the Agility used to dodge falls to a third, then to nothing past about twice Endurance + Strength. |
| `ui/sheetview.cpp:198` | ; leaves the party in %s |
| `ui/sheetview.cpp:207` | Darklands+ ("The best member speaks for the party", F12 > Settings > Cards and towns): on cards, each test is made by the member best at it, and shops set prices by the best bargainer, whoever leads. The leader is still the one meant by "the leader" on a card (a mine's traps, a church's favor, an item handed over), and in battles and dungeons (when not paused) deals with doors, chests, locks and traps and leads the group's movement. |
| `ui/sheetview.cpp:210` | The leader speaks and acts for the party on the cards and bargains in the shops. |
| `ui/sheetview.cpp:239` | a little slower (the attack rate falls) |
| `ui/sheetview.cpp:239` | much slower (the attack rate falls a lot) |
| `ui/sheetview.cpp:239` | at the usual pace |
| `ui/sheetview.cpp:242` | Load %d: the weight of what %s has equipped (weapons, armor, shield). Items carried but not equipped weigh nothing. Compared with Endurance + Strength (%d now): light up to %d, normal up to %d, heavy up to %d, overloaded above that.  Now: - Tests on cards that ask for Agility (climbing a wall, running, a trap): Agility %d counts %s (%d). - Fending off blows in melee: Agility counts %s (%d). - Swinging in melee: %s.  Travel, walking in a battle, missiles and Endurance are not affected by the load. |
| `ui/sheetview.cpp:346` |  when equipped |
| `ui/sheetview.cpp:348` | ; this member's mount |
| `ui/sheetview.cpp:350` |  in the party's pack |
| `ui/sheetview.cpp:350` | ; in the party's pack |
| `ui/sheetview.cpp:351` | ; kept (not sold or dropped) |
| `ui/sheetview.cpp:359` | To give the lot, drag it onto a member at the left, or press their number (1 to 5); Shift: just one. |
| `ui/sheetview.cpp:359` | To give it, drag it onto a member at the left, or press their number (1 to 5). |
| `ui/sheetview.cpp:364` | too weak by  |
| `ui/sheetview.cpp:365` |  can use it well |
| `ui/sheetview.cpp:369` | %s's defense with it: against a blow, %d + their melee rating; against a shot or throw, %d + twice the Agility %d = %d%s |
| `ui/sheetview.cpp:370` |  (the load lowers Agility) |
| `ui/sheetview.cpp:376` | Attack and defense hand to hand with  |
| `ui/sheetview.cpp:376` | these weapons |
| `ui/sheetview.cpp:387` | Point at an item to see it here; F11 shows what you can do. |
| `ui/sheetview.cpp:389` | Point at an item to see it here. Drag an item onto a member at the left to give it to them (Shift: just one of a stack). Click another member at the left to see their sheet; Esc or Close goes back. |
| `ui/sheetview.cpp:391` | Point at an attribute or a skill to see what it does and the whole party's values. Gold dot: the party's best at it; blue: raised by a saint for a while. |
| `ui/sheetview.cpp:392` | Point at an attribute or a skill to see what it does. Blue: raised by a saint for a while. |
| `ui/sheetview.cpp:396` | In effect now:  |
| `ui/sheetview.cpp:398` |  more (see "Blessings and potions" in the margin). |
| `ui/sheetview.cpp:423` | Worth about %s |
| `ui/sheetview.cpp:454` | Carries nothing. |
| `ui/sheetview.cpp:484` | The party's pack |
| `ui/sheetview.cpp:485` | The party's pack |
| `ui/sheetview.cpp:539` | Kept from selling and dropping (Sell spare gear skips it). Click to stop keeping it. |
| `ui/sheetview.cpp:540` | Keep it: Darklands+ will not sell it (one by one or with Sell spare gear) or drop it until you click again. |
| `ui/sheetview.cpp:571` | : drop it on a member at the left |
| `ui/sheetview.cpp:644` | : Virtue too low for this saint |
| `ui/sheetview.cpp:671` | Known in the party |
| `ui/sheetview.cpp:707` | No saint helps with that. |
| `ui/sheetview.cpp:707` | Nobody in the party knows a saint that helps with that. (All saints shows the others too.) |
| `ui/sheetview.cpp:748` | No library the party has been to teaches this saint. |
| `ui/sheetview.cpp:750` | Taught in the library of  |
| `ui/sheetview.cpp:773` | Nobody in the party knows this saint. |
| `ui/sheetview.cpp:777` | Asks a Virtue of %d. |
| `ui/sheetview.cpp:777` | Asks a Virtue of %d: too low for %s, who cannot call on this saint. |
| `ui/sheetview.cpp:789` | Call on this saint now: the prayer screen, where you choose how much Divine Favor to spend. |
| `ui/sheetview.cpp:796` | Differs from the description:  |
| `ui/sheetview.cpp:834` | The party has none. |
| `ui/sheetview.cpp:835` | The party has %d. |
| `ui/sheetview.cpp:836` | The party has %d (%s). |
| `ui/sheetview.cpp:838` | Where to buy: %s |
| `ui/sheetview.cpp:864` | All formulae |
| `ui/sheetview.cpp:864` | Known in the party |
| `ui/sheetview.cpp:885` | Nobody in the party knows a formula. All formulae shows every one, its ingredients and where it is sold. |
| `ui/sheetview.cpp:927` | The party's ingredients |
| `ui/sheetview.cpp:957` | Mystic number %d, %s. |
| `ui/sheetview.cpp:957` | risk unknown |
| `ui/sheetview.cpp:960` | The mystic number: the higher, the harder the formula is to brew (set against the alchemist's Alchemy and Intelligence). The risk: how badly a failed brew can go wrong. |
| `ui/sheetview.cpp:965` | Ingredients: the party has / needed |
| `ui/sheetview.cpp:976` | Enough for %d batch%s. |
| `ui/sheetview.cpp:977` | The party lacks an ingredient: brewing must wait. |
| `ui/sheetview.cpp:977` | The party lacks an ingredient. |
| `ui/sheetview.cpp:979` | Chance of brewing one batch |
| `ui/sheetview.cpp:988` | Nobody in the party knows it. |
| `ui/sheetview.cpp:991` | No alchemist sells it: it is traded for or found. |
| `ui/sheetview.cpp:992` | No alchemist the party has been to sells it. |
| `ui/sheetview.cpp:997` | Sold by the alchemist of  |
| `ui/skillicons.cpp:413` | Frames and clock |
| `ui/skillicons.cpp:417` | Cards and towns |
| `ui/skillicons.cpp:422` | The party and its items |
| `ui/skillicons.cpp:438` | Fixes for bugs of the original |
| `ui/skillicons.cpp:489` | Frames and clock |
| `ui/skillicons.cpp:489` | Cards and towns |
| `ui/skillicons.cpp:489` | The party and its items |
| `ui/skillicons.cpp:490` | Fixes for bugs of the original |
| `ui/skillicons.cpp:512` | C. A laurel wreath, larger |
| `ui/skillicons.cpp:559` | The four bases: the elements' triangles |
| `ui/skillicons.cpp:560` | Metals and their ores: the planets' signs, in pewter (gold in gold) |
| `ui/skillicons.cpp:561` | Sulphurs, salts and poisons |
| `ui/skillicons.cpp:562` | Waters, oils and vapours |
| `ui/skillicons.cpp:563` | Herbs: no sign in the tables, one made as theirs are |
| `ui/skillicons.cpp:564` | E. Black Bean |
| `ui/skillicons.cpp:564` | White Cinnabar |
| `ui/skillicons.cpp:566` | Choleric Base |
| `ui/skillicons.cpp:566` | Phlegmatic Base |
| `ui/skillicons.cpp:566` | Sanguine Base |
| `ui/skillicons.cpp:566` | Melancholic Base |
| `ui/skillicons.cpp:569` | Each ingredient: its sign now (left), and the picture it had before (right) |
| `ui/skillicons.cpp:598` | Drawn in color (no disc) |
| `ui/teachersui.cpp:47` | Missile Devices |
| `ui/teachersui.cpp:48` | Speak Common |
| `ui/teachersui.cpp:48` | Read & Write |
| `ui/teachersui.cpp:64` | Teaches: nothing that can be learned.  |
| `ui/teachersui.cpp:74` |  per student per day.  |
| `ui/teachersui.cpp:78` | Lessons go up to %d, the teacher's own ability, and no further (a Darklands+ fix; in the original game students could reach 99).  |
| `ui/teachersui.cpp:80` | The teacher's own ability is %d, but lessons go past it: students can reach 99, as in the original game.  |
| `ui/teachersui.cpp:82` | Each day of lessons: %s %d%% chance of +1 to the skill (about one point every %d day%s).  |
| `ui/teachersui.cpp:85` |  How to hire:  |
| `ui/teachersui.cpp:85` |  The teacher then stays in town for a week; at the inn, set a member to "Train or study". |
| `ui/teachersui.cpp:86` |  Who they accept:  |
| `ui/teachersui.cpp:87` |  Right now:  |
| `ui/teachersui.cpp:99` | %s (until %d %s) |
| `ui/tips.cpp:56` | Never show again (this will nag you %d more time%s) |
| `ui/tips.cpp:57` | Never show again |
| `ui/tips.cpp:118` | New to Darklands? |
| `ui/tips.cpp:119` | Darklands came with a thick manual and explains little by itself. The New Player Guide sums up how it works: the first steps, the party, money, towns, healing, alchemy, saints, battle, travel and saving, with the advice players have long given one another. |
| `ui/tips.cpp:122` | It is in the Darklands+ window: {START} > New Player Guide. |
| `ui/tips.cpp:123` | It is in the Darklands+ window at any time: F12 > New Player Guide. F11 lists the keys of the screen in view. |
| `ui/tips.cpp:125` |   Read the New Player Guide   |
| `ui/tips.cpp:143` | Battles, with Improved combat |
| `ui/tips.cpp:144` | Your people fight on between orders |
| `ui/tips.cpp:145` | A member with a bow, crossbow, gun or thrown weapon keeps shooting; when the target falls, at the nearest enemy in sight. |
| `ui/tips.cpp:146` | With no clear line (a wall, a tree or a friend in the way), a shooter steps a few paces aside to find one. |
| `ui/tips.cpp:147` | Shooters draw a melee weapon once an enemy is close, when nothing is left to shoot, or to help a friend nearby who is set upon or badly hurt. |
| `ui/tips.cpp:148` | A member whose enemy falls turns on another close by; one with no orders joins a fight beside it, unless told to hold. |
| `ui/tips.cpp:150` | {START} > New Player Guide > Battle explains the rest; F11 on a keyboard lists the keys. |
| `ui/tips.cpp:151` | Space pauses and resumes. Orders can be given while paused. |
| `ui/tips.cpp:153` | Click a member, then an enemy, a spot or a menu choice; Enter gives the order. |
| `ui/tips.cpp:154` | A click on an enemy lets your member choose the attack: a Vulnerable one, aimed at the gaps, when only that gets through the enemy's armor; else a standard one. An attack you pick from the orders yourself (Std Attack, Vulnerable, Berserk, Parry) is kept. |
| `ui/tips.cpp:155` | F11 lists every key |
| `ui/tips.cpp:156` | ; Tab, the level map |
| `ui/tips.cpp:160` | Std Attack: blows and guard in balance. |
| `ui/tips.cpp:161` | Vulnerable: slow blows aimed at the gaps in the armor; they pierce more (1 to 4 more penetration), but come far less often and land a little less. For armor the weapon cannot get through. |
| `ui/tips.cpp:162` | Berserk: quick, sure blows, without thought of defense. To finish a foe fast. |
| `ui/tips.cpp:163` | Parry: mostly defends; hard to hit, striking back slower and less surely. To hold a foe off while help comes, or to spare a wounded member. |
| `ui/tips.cpp:165` | Improved combat can be switched off in F12 > Settings > Battles. |
| `ui/tips.cpp:167` |   To battle   |
| `ui/titleview.cpp:26` | saved today at %d:%02d |
| `ui/titleview.cpp:27` | saved yesterday at %d:%02d |
| `ui/titleview.cpp:28` | saved %d %s %d |
| `ui/titleview.cpp:66` | Medieval Germany, the fifteenth century |
| `ui/titleview.cpp:81` | %d saved game%s to choose from (The Story Continues) |
| `ui/titleview.cpp:82` | Load a saved game |
| `ui/titleview.cpp:83` | Load a saved game |
| `ui/titleview.cpp:83` | No saved games yet |
| `ui/titleview.cpp:84` | Create a New World: up to four characters of your own |
| `ui/titleview.cpp:85` | Start at once with four ready-made characters |
| `ui/titleview.cpp:86` | Darklands+ settings, mods, New Player Guide (F12) |
| `ui/titleview.cpp:87` | Back to the desktop |
| `ui/titleview.cpp:104` | Enter continues;  L load,  N new party,  Q Quickstart |
| `ui/titleview.cpp:104` | L load,  N new party,  Q Quickstart |
| `ui/titleview.cpp:104` | Enter or N: a new party;  Q Quickstart |
| `ui/titleview.cpp:107` | Darklands+ %s (build %s) |
| `ui/titleview.cpp:107` | Darklands+ build %s%s |
| `ui/townicons.cpp:223` | von der Lilie |
| `ui/ui.cpp:84` |              |
| `ui/ui.cpp:97` | EB Garamond (book face) |
| `ui/ui.cpp:97` | Alegreya (calligraphic) |
| `ui/ui.cpp:97` | IM Fell English (old print) |
| `ui/ui.cpp:98` | Atkinson Hyperlegible (easiest to read) |
| `ui/ui.cpp:98` | MedievalSharp (medieval hand) |
| `ui/ui.cpp:98` | Fraktur (blackletter, for the brave) |
| `ui/ui.cpp:99` | Grenze Gotisch (gothic, still readable) |
| `ui/ui.cpp:99` | Comic Sans (we are so sorry) |
| `ui/ui.cpp:224` | Comic Neue (a Comic Sans look-alike, we are so sorry) |
| `ui/ui.cpp:241` | don't remind me |
| `ui/ui.cpp:260` | Darklands stopped because of an error (see darklands_port.log; F12 > About says where it is). Your saves are safe: Load is in the Game menu (right-click). |
| `ui/ui.cpp:261` | The party's journey has ended. Load a saved game to go on: Load in the Game menu (right-click). |
| `ui/ui.cpp:303` | The world map can only be opened from the overland view or a card, not in a battle. |
| `ui/ui.cpp:304` | The world map can only be opened from the overland view or a card: close this screen first. |
| `ui/ui.cpp:305` | The world map can only be opened from the overland view or a card, once a game is under way. |
| `ui/ui.cpp:414` | How Darklands works |
| `ui/ui.cpp:422` | A deep editor, for fixing and modding: |
| `ui/ui.cpp:424` | it changes the saved game's own data directly, as the community's save editors do (to repair a character hit by an old bug, to cheat, or to make a mod). Nothing here is needed to play. |
| `ui/ui.cpp:428` | Items (for a mod) |
| `ui/ui.cpp:473` | Darklands+: the potions and the alchemical ingredients |
| `ui/ui.cpp:475` | Potions, grades 1, 2 and 3 (the gold dots on the neck) |
| `ui/ui.cpp:586` | New Player Guide |
| `ui/ui.cpp:587` | New Player Guide |
| `ui/ui.cpp:587` | Reconstructed Content |
| `ui/ui.cpp:591` | Reconstructed Content |
| `ui/ui.cpp:592` | New Player Guide |
| `ui/ui.cpp:613` |              |
| `ui/ui.cpp:705` | Darklands+ is preparing the game from your own DARKLAND.EXE. This happens only once and takes a minute or two. |
| `ui/ui.cpp:711` | Nothing from the game is included in the Darklands+ download; it is built here from your own copy. |
| `ui/ui_windows.cpp:74` | Darklands (c) 1992 MicroProse. This is the original game, rebuilt to run directly on today's computers, with the game files of your own copy. F12 > Settings has the Full Original preset, the game as it was, and switches for most of what Darklands+ adds. |
| `ui/ui_windows.cpp:77` | F11: the keys for the current screen.  F12: this window (the game waits while it is open).  Alt+Enter: full screen.  Ctrl+F12: screenshot.  Ctrl+S: quick save.  F9 or Ctrl+L: quick load.  M: world map.  G: go to (in cities). |
| `ui/ui_windows.cpp:81` | saved games, and every other file the game writes: %s (Darklands+ cannot write to the game folder; the saved games in its SAVES folder can still be loaded) |
| `ui/ui_windows.cpp:83` | saved games: the SAVES folder of the game folder, as in the original |
| `ui/ui_windows.cpp:86` | the game: %s |
| `ui/ui_windows.cpp:87` | settings: darklands_port.ini in %s |
| `ui/ui_windows.cpp:87` | the game folder |
| `ui/ui_windows.cpp:89` | mods, replacement art and sounds: the mods, art and sounds folders in %s |
| `ui/ui_windows.cpp:89` | the game folder |
| `ui/ui_windows.cpp:90` | screenshots (Ctrl+F12, the window as you see it): the screenshots folder in %s |
| `ui/ui_windows.cpp:90` | the game folder |
| `ui/ui_windows.cpp:91` | the session log: darklands_port.log (Windows) in %s; send it along when you report a problem |
| `ui/ui_windows.cpp:91` | the game folder |
| `ui/ui_windows.cpp:93` | the game's program, made from your DARKLAND.EXE on the first start: %s (delete it to have it made again) |
| `ui/ui_windows.cpp:102` | Dear ImGui (MIT license); Nuked OPL3, the Sound Blaster FM synthesis (LGPL 2.1); Munt's libmt32emu, the Roland MT-32 (LGPL 2.1); stb_image, stb_image_write, stb_vorbis (public domain); SDL2 (zlib license); fonts EB Garamond, Alegreya, Atkinson Hyperlegible, IM Fell English, MedievalSharp, UnifrakturMaguntia, Grenze Gotisch and Comic Neue (SIL Open Font License 1.1). |
| `ui/ui_windows.cpp:105` | Darklands was designed by Arnold Hendrick and made by MicroProse; everything here stands on their game. |
| `ui/ui_windows.cpp:106` | With thanks to the Darklands community: vvendigo's tools, Merle's and Joel "Quadko" McIntyre's documentation of the file formats, the Darklands Companion, and Arkana Mechanika (Darklands Restoration) for bug reports. |
| `ui/ui_windows.cpp:121` | Welcome to Darklands+ |
| `ui/ui_windows.cpp:124` | This is Darklands (MicroProse, 1992), rebuilt to run directly on today's computers, with the game files of your own copy. Windows does not let programs write to the game folder where it is installed, so your saved games go to %s (the ones already in the game folder can be loaded as always). |
| `ui/ui_windows.cpp:128` | This is Darklands (MicroProse, 1992), rebuilt to run directly on today's computers, with the game files of your own copy. Your saved games stay in the game folder as always. |
| `ui/ui_windows.cpp:131` | Darklands+ adds what players have long wished for: crisp text, chances on the cards, a character sheet for the whole party, journals, a world map, a battle log, quick saves and fixes for old bugs. Choose how much of it below; most of it can also be switched on or off one by one. |
| `ui/ui_windows.cpp:136` | How would you like to play? |
| `ui/ui_windows.cpp:141` |  (recommended) |
| `ui/ui_windows.cpp:152` | one save, kept by the game itself as you go; no loading back. For your next new game, with any of the three. |
| `ui/ui_windows.cpp:156` | Saved games stay compatible with the original game whichever you choose. |
| `ui/ui_windows.cpp:157` | Change it at any time, or turn single features on or off: {START} > Settings. {LSTICK} moves the pointer, {A} clicks. |
| `ui/ui_windows.cpp:158` | Change it at any time, or turn single features on or off: F12 > Settings. |
| `ui/ui_windows.cpp:163` |   Read the New Player Guide first   |
| `ui/ui_windows.cpp:168` | F12: the Darklands+ window (the game waits while it is open) |
| `ui/ui_windows.cpp:169` | F11: the keys and clicks for the current screen, including the game's hidden ones |
| `ui/ui_windows.cpp:170` | F12 > New Player Guide: how Darklands works, for a new player |
| `ui/ui_windows.cpp:171` | Click a party member on the left: their character sheet, with the whole party's saints and formulae |
| `ui/ui_windows.cpp:172` | M: the world map, to find a place and travel there |
| `ui/ui_windows.cpp:173` | Right-click: the game's menu bar (save, load, orders in battle) |
| `ui/ui_windows.cpp:174` | Alt+Enter: full screen   Ctrl+S: quick save   F9: quick load |
| `ui/ui_windows.cpp:200` | New in Darklands+  |
| `ui/ui_windows.cpp:203` | The Darklands+ window (F12) has fewer, clearer tabs: Journal, Settings, Reconstructed Content, Mods, Editor, New Player Guide, About. Settings and the New Player Guide list their sections on the left. |
| `ui/ui_windows.cpp:205` | The quests are on the world map (M), grouped by kind and nearest first, with a button for where to buy ingredients and wares. |
| `ui/ui_windows.cpp:206` | Saving and loading happen in the game's own Save and Load screens, drawn anew, with their options there. |
| `ui/ui_windows.cpp:207` | Improved combat: between your orders, shooters keep shooting, step aside for a clear line, and fight hand to hand to help a friend set upon; a member with no orders joins a fight beside it. The first battle explains it. |
| `ui/ui_windows.cpp:209` | On a card, beside each chance: the head of who tries it, and what it rests on (skills, attributes, the town's opinion, fame). |
| `ui/ui_windows.cpp:210` | Ironman: one save, kept by the game itself as you go, no loading back (F12 > Settings > Preset, for your next new game). |
| `ui/ui_windows.cpp:211` | The whole soundtrack can be played in F12 > Settings > Sound, on the device you choose. |
| `ui/ui_windows.cpp:212` | More screens drawn anew: the inn and the camp, the alchemist's desk, Party Info, the marching order (drag the members into place), enemy info. |
| `ui/ui_windows.cpp:213` | City politics, cities at war and other things MicroProse wrote and never switched on, reconstructed by Darklands+ (experimental, all off until you tick them): F12 > Reconstructed Content. |
| `ui/ui_windows.cpp:215` | A controller's buttons are shown on screen and can be changed: F12 > Settings > Controller. |
| `ui/ui_windows.cpp:216` | Your saved games are copied into "Darklands+ save backups" the first time each new version starts. |
| `ui/ui_windows.cpp:220` | Most changes can be switched off in F12 > Settings (the Full Original preset turns them all off). F12 > New Player Guide explains the game; F11 lists the keys for the current screen. |
| `ui/ui_windows.cpp:223` |   Back to the game   |
| `ui/ui_windows.cpp:240` | Darklands sounds best on a Roland MT-32 |
| `ui/ui_windows.cpp:241` | Darklands was written for the Roland MT-32, a synthesizer of 1987 (or its sibling the LAPC-I, a card inside the computer). Everything the game plays goes through it as notes, the battle sounds too. The game first loads instruments of its own into it, so the music comes out as composed, with flutes, strings and horns, and the battles sound like swords and shields... sort of: each blow is still a synthesizer playing a note. |
| `ui/ui_windows.cpp:245` | Every other sound setting is a stand-in. The Sound Blaster, the usual sound card of the time, plays simpler versions of the music and rougher imitations of the battle sounds; an ordinary synthesizer, such as the one built into Windows, plays the wrong instruments altogether. |
| `ui/ui_windows.cpp:249` | Darklands+ has an MT-32 built in (Munt's imitation of it). It only needs two files copied from a real one's memory, its ROM files: control and PCM, of an MT-32 or of the later CM-32L (either works; a CM-32L turns the battle sounds into chimes, so with both, play on the MT-32). They are usually named MT32_CONTROL.ROM and MT32_PCM.ROM (CM32L_CONTROL.ROM and CM32L_PCM.ROM for a CM-32L); any names do, Darklands+ knows them by what is in them. They belong to Roland, so they cannot come with this download. If you play other games with Munt or DOSBox and an MT-32, you have them already: show Darklands+ the folder. |
| `ui/ui_windows.cpp:263` | Choose the folder with the ROM files... |
| `ui/ui_windows.cpp:265` | The folder with the MT-32 or CM-32L ROM files (control and PCM): |
| `ui/ui_windows.cpp:267` | No MT-32 or CM-32L ROM files in that folder. |
| `ui/ui_windows.cpp:275` | No ROM files found there yet. |
| `ui/ui_windows.cpp:277` | Darklands+ also looks in Munt's ROM folder, DOSBox Staging's mt32-roms folder, and a "roms" folder in the Darklands+ folder or the game folder. |
| `ui/ui_windows.cpp:282` | Restart with the MT-32 |
| `ui/ui_windows.cpp:288` | F12 > Settings > Sound has all of this too. |
| `ui/ui_windows.cpp:304` | Keys and clicks (F11) |
| `ui/ui_windows.cpp:317` | Now: loading a game |
| `ui/ui_windows.cpp:317` | Now: saving the game |
| `ui/ui_windows.cpp:318` | Double-click a save |
| `ui/ui_windows.cpp:318` | load it (or click it, then Load) |
| `ui/ui_windows.cpp:318` | Up, Down, Enter |
| `ui/ui_windows.cpp:318` | choose a save, load it |
| `ui/ui_windows.cpp:319` | a name for the save (22 characters at most; Backspace deletes one) |
| `ui/ui_windows.cpp:319` | Enter or Save |
| `ui/ui_windows.cpp:320` | Esc or Cancel |
| `ui/ui_windows.cpp:320` | back to the game as it was |
| `ui/ui_windows.cpp:322` | Now: a stay at the inn |
| `ui/ui_windows.cpp:323` | A member, or 1 ... 5 |
| `ui/ui_windows.cpp:323` | choose the member whose day you are planning |
| `ui/ui_windows.cpp:324` | J R P A E G T |
| `ui/ui_windows.cpp:324` | Regain Endurance (Just Relax), Regain Strength, Pray, Alchemy work, Earn a little money, Guard the camp, Train or study (the original's keys) |
| `ui/ui_windows.cpp:325` | one day passes; A week: seven days; Until rested: members short of Strength who only relax switch to Regain Strength, then days pass until nobody is regaining Strength or praying |
| `ui/ui_windows.cpp:326` | L, Esc or Leave |
| `ui/ui_windows.cpp:329` | Click an option |
| `ui/ui_windows.cpp:329` | or press its number, 1 to 9 |
| `ui/ui_windows.cpp:330` | Hold Shift over an option |
| `ui/ui_windows.cpp:330` | the chance of success as the original game shows it (Darklands+'s badges show every option's chance) |
| `ui/ui_windows.cpp:331` | go to a place in this city (Darklands+) |
| `ui/ui_windows.cpp:332` | the world map (Darklands+) |
| `ui/ui_windows.cpp:333` | (in the right margin) what this town has that most towns lack, and its notable shops: point at one to see what it offers |
| `ui/ui_windows.cpp:335` | Now: the overland view |
| `ui/ui_windows.cpp:336` | walk towards that spot |
| `ui/ui_windows.cpp:337` | travel there along the roads (Darklands+; any key or click stops) |
| `ui/ui_windows.cpp:338` | the world map (Darklands+): find a place, double-click to travel there |
| `ui/ui_windows.cpp:339` | Point at a quest's icon |
| `ui/ui_windows.cpp:339` | the quest that leads there (a portcullis or a pillory beside a town: the town bars the party, or its watch wants the party) |
| `ui/ui_windows.cpp:340` | A quest in the left margin |
| `ui/ui_windows.cpp:340` | click to open it, then Travel there |
| `ui/ui_windows.cpp:341` | (in the right margin, near a town) what that town has that most towns lack, and its notable shops (point at one to see what it offers) |
| `ui/ui_windows.cpp:343` | Now: a battle |
| `ui/ui_windows.cpp:344` | pause / resume; orders can be given while paused |
| `ui/ui_windows.cpp:346` | Click a member |
| `ui/ui_windows.cpp:346` | then click an enemy (attack) or a spot (walk there): that click gives the order |
| `ui/ui_windows.cpp:347` | select a member; Esc: cancel the order being given |
| `ui/ui_windows.cpp:350` | The right margin's buttons |
| `ui/ui_windows.cpp:350` | the other orders (Vulnerable, Berserk, Parry, Use Missile, Halt ...); for a member already fighting, an attack is given against that enemy at once |
| `ui/ui_windows.cpp:352` | Click a member |
| `ui/ui_windows.cpp:352` | then click an enemy, a spot or a menu choice; Enter gives the order |
| `ui/ui_windows.cpp:353` | Ctrl+click a member |
| `ui/ui_windows.cpp:353` | make that member the active one: the view follows them (scrolling at an edge stops it) |
| `ui/ui_windows.cpp:354` | Point at anyone |
| `ui/ui_windows.cpp:354` | who it is and how badly hurt |
| `ui/ui_windows.cpp:355` | a map of the level as far as the party has seen it (Darklands+) |
| `ui/ui_windows.cpp:357` | The battle log |
| `ui/ui_windows.cpp:357` | every blow and wound: the left margin on a wide window, else the bottom right; F12 > Journal > Battle log keeps it all |
| `ui/ui_windows.cpp:359` | Now: the loot |
| `ui/ui_windows.cpp:360` | Take / Put back |
| `ui/ui_windows.cpp:360` | on the row under the pointer: the whole stack (1: just one) |
| `ui/ui_windows.cpp:361` | everything into the party's pack |
| `ui/ui_windows.cpp:361` | go to the member shown above the right list (each member's own); the rest into the party's pack |
| `ui/ui_windows.cpp:362` | Take all / Share out |
| `ui/ui_windows.cpp:362` | everything to the member shown / to whoever has some already, else whoever has the most room (A / S) |
| `ui/ui_windows.cpp:363` | Names above the right list, D |
| `ui/ui_windows.cpp:363` | switch to another member |
| `ui/ui_windows.cpp:364` | L, Esc or Leave |
| `ui/ui_windows.cpp:364` | leave (what is left stays behind) |
| `ui/ui_windows.cpp:366` | Now: the loot |
| `ui/ui_windows.cpp:367` | get the chosen item from the pile / put one back (the cursor's list) |
| `ui/ui_windows.cpp:368` | switch to the next member (the one who gets what you take) |
| `ui/ui_windows.cpp:369` | take the whole pile for the member shown |
| `ui/ui_windows.cpp:369` | share the pile out: each item to whoever has some already, else whoever has the most room |
| `ui/ui_windows.cpp:370` | leave (what is left stays behind) |
| `ui/ui_windows.cpp:373` | on the row under the pointer; with Shift: ten / the whole stack |
| `ui/ui_windows.cpp:374` | Names above your list |
| `ui/ui_windows.cpp:374` | whose items are listed (B: the next member); right-click a name: that member holds the purse |
| `ui/ui_windows.cpp:375` | Wheel, Up, Down |
| `ui/ui_windows.cpp:375` | scroll the list under the pointer |
| `ui/ui_windows.cpp:376` | L, Esc or Leave |
| `ui/ui_windows.cpp:379` | buy the merchant's chosen item / sell your chosen item (one at a time) |
| `ui/ui_windows.cpp:380` | buy ten of the merchant's chosen item (while the money lasts) |
| `ui/ui_windows.cpp:380` | sell the whole stack of your chosen item |
| `ui/ui_windows.cpp:381` | barter for the next member |
| `ui/ui_windows.cpp:384` | Now: the store room |
| `ui/ui_windows.cpp:385` | on the row under the pointer: the whole stack (1: just one) |
| `ui/ui_windows.cpp:386` | every item not equipped (with the party's pack, Darklands+: except kept items and potions) |
| `ui/ui_windows.cpp:387` | a potion taken goes to the member shown above the right list; the rest into the party's pack |
| `ui/ui_windows.cpp:388` | The store room |
| `ui/ui_windows.cpp:388` | the same at every inn (Darklands+; F12 > Settings > New screens) |
| `ui/ui_windows.cpp:389` | L, Esc or Leave |
| `ui/ui_windows.cpp:391` | Now: a character sheet |
| `ui/ui_windows.cpp:392` | Equip / Unequip / Drink / Keep / Drop |
| `ui/ui_windows.cpp:392` | on the row under the pointer on the Items page: with the shared pack (Darklands+) the spare items are in the party's pack, and Equip moves the item from the pack to this member and equips it (Drop with Shift: all of them) |
| `ui/ui_windows.cpp:393` | each member's own: drag one onto someone at the left to give it |
| `ui/ui_windows.cpp:394` | marks an item never to be sold or dropped (a small seal on its row) |
| `ui/ui_windows.cpp:395` | only equipped items count toward the load, as in the original |
| `ui/ui_windows.cpp:396` | Equip / Unequip / Drink / Give / Drop |
| `ui/ui_windows.cpp:396` | on the row under the pointer on the Items page (Give and Drop with Shift: the whole stack) |
| `ui/ui_windows.cpp:397` | Items, Skills, Saints, Formulae |
| `ui/ui_windows.cpp:397` | the pages at the top (Tab or Right: next page; Shift+Tab or Left: previous page) |
| `ui/ui_windows.cpp:398` | Portraits, F1 ... F5 |
| `ui/ui_windows.cpp:398` | another member's sheet; this member's own portrait, Esc or Close: back |
| `ui/ui_windows.cpp:399` | Ctrl+F1 ... Ctrl+F5 |
| `ui/ui_windows.cpp:399` | make member 1 ... 5 the leader (or Make leader) |
| `ui/ui_windows.cpp:400` | Wheel, Up, Down |
| `ui/ui_windows.cpp:400` | scroll the list |
| `ui/ui_windows.cpp:402` | Now: a character sheet |
| `ui/ui_windows.cpp:403` | Ctrl+F1 ... Ctrl+F5 |
| `ui/ui_windows.cpp:403` | make member 1 ... 5 the leader (or Make leader at the top) |
| `ui/ui_windows.cpp:404` | Up, Down, PgUp, PgDn |
| `ui/ui_windows.cpp:404` | move in a list (the mouse wheel too) |
| `ui/ui_windows.cpp:405` | in the equipment list: give the chosen item to member 1 ... 5 (with Shift: all of them) |
| `ui/ui_windows.cpp:406` | in the equipment list: ready or wear the chosen item |
| `ui/ui_windows.cpp:407` | in the equipment list: drop the chosen item (with Shift: all of them) |
| `ui/ui_windows.cpp:409` | Keys of the original game |
| `ui/ui_windows.cpp:410` | the menu bar: click an entry (or hold the button and let go on it); a click elsewhere or any key closes it; F10 with the keyboard |
| `ui/ui_windows.cpp:411` | Right mouse button |
| `ui/ui_windows.cpp:411` | the menu bar, while it is held: let go on an entry (F10 with the keyboard) |
| `ui/ui_windows.cpp:412` | Alt+S / Alt+L |
| `ui/ui_windows.cpp:412` | save / load, with a comment (the original's screens list nine saves) |
| `ui/ui_windows.cpp:413` | the marching order |
| `ui/ui_windows.cpp:414` | difficulty    Alt+M music    Alt+F sound effects |
| `ui/ui_windows.cpp:415` | messages about saints' changes on / off |
| `ui/ui_windows.cpp:417` | quit to the desktop |
| `ui/ui_windows.cpp:419` | the Darklands+ window (pauses the game) |
| `ui/ui_windows.cpp:420` | Ctrl+S / F9 or Ctrl+L |
| `ui/ui_windows.cpp:420` | quick save / quick load |
| `ui/ui_windows.cpp:421` | full screen    Ctrl+F12 screenshot |
| `ui/ui_windows.cpp:424` | The controller |
| `ui/ui_windows.cpp:430` | moves the pointer (slowly while the left trigger is held) |
| `ui/ui_windows.cpp:431` | the mouse wheel; pressed: Z (fast forward in a battle) |
| `ui/ui_windows.cpp:432` | click / right-click (the menu bar) |
| `ui/ui_windows.cpp:434` | the arrow keys |
| `ui/ui_windows.cpp:435` | Tab (the level map) / Space (pause) |
| `ui/ui_windows.cpp:436` | {START} {BACK} |
| `ui/ui_windows.cpp:436` | F12 (the Darklands+ window) / M (the world map) |
| `ui/ui_windows.cpp:437` | held: Ctrl (in the overland view: Ctrl+click travels there) |
| `ui/ui_windows.cpp:438` | pressed: F11 (this list) |
| `ui/ui_windows.cpp:440` | F11, Esc or a click closes this. |
| `ui/worldmap.cpp:86` | a university |
| `ui/worldmap.cpp:86` | an alchemist |
| `ui/worldmap.cpp:87` | docks (boats) |
| `ui/worldmap.cpp:88` | a swordsmith of 30q+ |
| `ui/worldmap.cpp:88` | an armorer of 30q+ |
| `ui/worldmap.cpp:88` | a bowyer of 30q+ |
| `ui/worldmap.cpp:89` | an artificer of 30q+ |
| `ui/worldmap.cpp:89` | Artificer 30q+ |
| `ui/worldmap.cpp:107` | everyone knows the swordsmiths of Nuremberg |
| `ui/worldmap.cpp:143` | Route planned: the party sets off once it is in the overland view. |
| `ui/worldmap.cpp:207` | Its saints, formulae and teachers: known once the party has been there. |
| `ui/worldmap.cpp:209` | The saints its library teaches: known once the party has been there. |
| `ui/worldmap.cpp:219` | Saints taught here: |
| `ui/worldmap.cpp:226` | Formulae sold here: |
| `ui/worldmap.cpp:235` | Teachers to be hired: |
| `ui/worldmap.cpp:239` | Not visited yet |
| `ui/worldmap.cpp:251` | Quest markers |
| `ui/worldmap.cpp:251` | Boat routes between the towns' docks |
| `ui/worldmap.cpp:251` | Each place as an icon of its kind (else a dot) |
| `ui/worldmap.cpp:252` | The days of travel from the party to every town, under its name (off at first: it crowds the map) |
| `ui/worldmap.cpp:461` | Towns with ... |
| `ui/worldmap.cpp:539` | the marked spot |
| `ui/worldmap.cpp:555` | Click for what it offers, and to travel there |
| `ui/worldmap.cpp:557` | Unwelcome here |
| `ui/worldmap.cpp:557` | Wanted by the watch here |
| `ui/worldmap.cpp:591` | No way there from here |
| `ui/worldmap.cpp:592` | Travel there |
| `ui/worldmap.cpp:597` | Travel there |
| `ui/worldmap.cpp:600` | Leave town and travel there |
| `ui/worldmap.cpp:605` | To travel, be in the overland view or in a town's streets |
| `ui/worldmap.cpp:611` | Walk out of %s's gate and travel to %s? |
| `ui/worldmap.cpp:612` | The party walks through the streets to a gate and out; in the overland view the journey begins. |
| `ui/worldmap.cpp:613` | Leave and travel |
| `ui/worldmap.cpp:632` | The wares noted here (%d shop%s) |
| `ui/worldmap.cpp:655` | Boats leaving here for: |
| `ui/worldmap.cpp:656` | Its docks have no boats leaving (the original game has none either). |
| `ui/worldmap.cpp:657` | Boats arriving here from: |
| `ui/worldmap.cpp:658` | Not every boat sails every day; the docks say which leave today. |
| `ui/worldmap.cpp:661` | Unwelcome here |
| `ui/worldmap.cpp:661` | Wanted by the watch here |
| `ui/worldmap.cpp:680` | The Holy Roman Empire |
| `ui/worldmap.cpp:681` | {LSTICK} moves the pointer, hold {A} to drag, {RSTICK} zooms. {BACK} or {Y} closes the world map. |
| `ui/worldmap.cpp:682` | Drag to move, wheel to zoom. M or Esc closes. |
| `ui/worldmap.cpp:684` | Traveling to %s |
| `ui/worldmap.cpp:708` | Collect your reward |
| `ui/worldmap.cpp:708` | Robber knights |
| `ui/worldmap.cpp:708` | Treasures to fetch |
| `ui/worldmap.cpp:708` | Papers to get in a town |
| `ui/worldmap.cpp:708` | City politics |
| `ui/worldmap.cpp:708` | Cities at war |
| `ui/worldmap.cpp:708` | The Wild Hunt |
| `ui/worldmap.cpp:708` | Other quests |
| `ui/worldmap.cpp:726` | %s (%d)###qg |
| `ui/worldmap.cpp:732` | Sorted %s first: click for %s first. |
| `ui/worldmap.cpp:732` | the soonest due |
| `ui/worldmap.cpp:741` | No quests or rumors yet. |
| `ui/worldmap.cpp:784` | cave near Köln |
| `ui/worldmap.cpp:790` | and %d more: type more of the name to narrow the list |
| `ui/worldmap.cpp:791` | No place of that name. |
| `ui/worldmap.cpp:821` | Where to buy ingredients and wares |
| `ui/worldmap.cpp:827` | Places on the map |
| `ui/worldmap.cpp:829` | Click a kind to show or hide it. |
| `ui/worldmap.cpp:850` | Click to find the party on the map. |
| `ui/worldmap.cpp:854` | A town not visited yet |
| `ui/worldmap.cpp:862` | Click one to circle those towns on the map. |
| `ui/worldmap.cpp:874` |  the party has been to: |
| `ui/worldmap.cpp:878` | ... and %d more (circled on the map) |
| `ui/worldmap.cpp:879` | %s %dq##tl%d |
| `ui/worldmap.cpp:890` | Show on the map |
| `ui/worldmap.cpp:896` | Show on the map |
| `ui/worldmap.cpp:918` | shown (click to hide) |
| `ui/worldmap.cpp:918` | hidden (click to show) |
| `ui/worldmap.cpp:989` | faded: leaves later this week |
| `ui/worldmap.cpp:1053` | Click to take the boat. |
| `ui/worldmap_draw.cpp:78` | ruined castle |
| `ui/worldmap_draw.cpp:80` | ruined village |
| `ui/worldmap_draw.cpp:81` | standing stone |
| `ui/worldmap_draw.cpp:81` | witches' meeting place |
| `ui/worldmap_draw.cpp:110` | Spring or lake |
| `ui/worldmap_draw.cpp:332` | Map of Greater Germany |
| `ui/worldmap_draw.cpp:463` | About %s of travel |

## The game side (mods/)

| Where | Text |
|---|---|
| `mods/aging.c:83` | The years take their toll |
| `mods/aging.c:84` | These losses are permanent, as with aging at character creation: from age 30, a little each month, and more as the years go by. (Aging during play is a Darklands+ switch: F12 > Settings > The party and its items.) |
| `mods/alchemyview.c:171` | Choose a formula and at least one batch first. |
| `mods/alchemyview.c:173` | %s starts %d batch%s. |
| `mods/assist.c:466` | keys %d depth %d polled %u ms ago ee41 %d ee42 %d options %d |
| `mods/autopause.c:73` | Paused: enemies come out to fight |
| `mods/autopause.c:88` | Paused: %s has fallen |
| `mods/autopause.c:88` | Paused: %s is badly hurt |
| `mods/barterview.c:152` | Not bought: the purse holds too little. |
| `mods/barterview.c:152` | Not bought: %s has no room for another kind of item. |
| `mods/barterview.c:153` | Bought %d %s. |
| `mods/barterview.c:161` | The merchants of the Empire will not buy that. |
| `mods/barterview.c:162` | Not sold: it is marked Keep on the character sheet (unmark it there first). |
| `mods/barterview.c:176` | Not sold: this merchant pays nothing for it. |
| `mods/barterview.c:184` | The merchants of the Empire will not buy that. |
| `mods/barterview.c:185` | Not sold: it is marked Keep on the character sheet (unmark it there first). |
| `mods/barterview.c:201` | Not sold: this merchant pays nothing for it. |
| `mods/barterview.c:297` | Nothing sold: this merchant pays nothing for those items. |
| `mods/barterview.c:301` | Sold the spare gear (%d kind%s of items) for %s. |
| `mods/battleend.c:30` | Victory! The battle is won. |
| `mods/battleend.c:30` | The party has retreated. |
| `mods/battleend.c:31` | The party is beaten. |
| `mods/battleend.c:31` | The party has surrendered. |
| `mods/battleend.c:32` | The whole party has fallen. |
| `mods/battleend.c:32` | The party has left the field. |
| `mods/builtin.c:103` | Faster overland travel |
| `mods/builtin.c:103` | Overland travel takes a third of the time. |
| `mods/builtin.c:104` | Half-price inns |
| `mods/builtin.c:104` | Lodging costs half as much. |
| `mods/builtin.c:105` | Quick learning |
| `mods/builtin.c:105` | Skills improve about twice as often through use, on cards and in battles (slow ones such as Streetwise and Latin too). |
| `mods/builtin.c:106` | Double damage against enemies |
| `mods/builtin.c:106` | Every hit on an enemy does double damage. Hits on the party are unchanged. |
| `mods/builtin.c:107` | Strong Quickstart party |
| `mods/builtin.c:107` | The four members of the Quickstart party start with 60 in every attribute and 75 in every skill. |
| `mods/builtin.c:108` | A party made with Create a New World starts with 5000 florins instead of 200. |
| `mods/cacheview.c:144` | %d %s to %s (to give it to another member, drag it from the character sheet onto them in the party panel). |
| `mods/cacheview.c:145` | %d %s into the party's pack. |
| `mods/cacheview.c:146` | No room: every member already carries 64 kinds of items. |
| `mods/cacheview.c:148` | %s takes %d %s. |
| `mods/cacheview.c:149` | %s has no room for another kind of item (64 at most). |
| `mods/cacheview.c:174` | %s stores %d %s. |
| `mods/cacheview.c:175` | The %s stays with %s. |
| `mods/cacheview.c:200` | %d %s stored. |
| `mods/cacheview.c:201` | The store room is full: the %s stays in the party's pack. |
| `mods/cacheview.c:218` | Stored %d item%s (%d kind%s); potions%s and equipped items are not stored. |
| `mods/cacheview.c:218` | , items marked Keep |
| `mods/cacheview.c:219` | Nothing to store: the party's pack holds only items marked Keep. |
| `mods/cacheview.c:219` | Nothing to store: the party's pack is empty. |
| `mods/cacheview.c:230` | %s stores %d item%s (%d kind%s); equipped items stay with them. |
| `mods/cacheview.c:231` | Nothing to store: everything %s carries is equipped. |
| `mods/cardview.c:125` | You gather around the comfortable fire at the |
| `mods/cardview.c:400` | \x81<name>\xff  |
| `mods/cutcards.c:68` | name     The hospice  |
| `mods/cutcards.c:68` | card     $CityH00  |
| `mods/cutcards.c:68` | picture  XDAMONK  |
| `mods/cutcards.c:68` | terrain  any  |
| `mods/cutcards.c:68` | vars     cost, wage  |
| `mods/cutcards.c:68` | chosen   bestskill(Healing)  |
| `mods/cutcards.c:68` | do       set cost = max(12, wounds(End) * 2 + wounds(Str) * 8); money2 cost; money1 240  |
| `mods/cutcards.c:79` |   need   money >= 240  |
| `mods/cutcards.c:79` |   chance none  |
| `mods/cutcards.c:79` |   ok     pay 240; text 1; attr party, DF, 1; reputation 1, 2; hours 1; disable 0  |
| `mods/cutcards.c:83` |   who    chosen  |
| `mods/cutcards.c:83` |   chance none  |
| `mods/cutcards.c:83` |   ok     if skill(chosen, Healing) < 20 goto refused else goto work  |
| `mods/cutcards.c:87` |   need   wounds(End) + wounds(Str) > 0 and money >= cost  |
| `mods/cutcards.c:87` |   chance none  |
| `mods/cutcards.c:87` |   ok     pay cost; hours 24; heal party, End, 100; heal party, Str, 100; text 4;  |
| `mods/cutcards.c:87` |          set cost = 12; money2 cost  |
| `mods/cutcards.c:92` |   need   wounds(End) + wounds(Str) > 0  |
| `mods/cutcards.c:92` |   who    bestattr(Cha)  |
| `mods/cutcards.c:92` |   chance clamp(5, 95, 40 + standing / 2 + maxattr(Cha) / 4 - money / 2400)  |
| `mods/cutcards.c:92` |   ok     hours 24; heal party, End, 100; heal party, Str, 50; text 5; disable 3;  |
| `mods/cutcards.c:92` |          set cost = max(12, wounds(End) * 2 + wounds(Str) * 8); money2 cost  |
| `mods/cutcards.c:92` |   fail   text 6; hours 1; disable 3  |
| `mods/cutcards.c:99` |   chance none  |
| `mods/cutcards.c:99` |   ok     card 0x13  |
| `mods/cutcards.c:102` | do       text 2; hours 1; disable 1  |
| `mods/cutcards.c:102` | do       set wage = 24 + skill(chosen, Healing); money1 wage; text 3; money1 240; gain wage  |
| `mods/cutcards.c:102` |          practice chosen, Healing, 1, 40, 0; practice party, Healing, 1, 10, 0; hours 24; disable 1  |
| `mods/cutcards.c:131` | ...\x1dvisit the Heilig-Geist-Spital, where injuries and the ill are tended. |
| `mods/datamods.c:360` | # right edge). Bytes the game uses for layout are written \\xx (hex): \\14 new paragraph, \\15 an option, # \\1d after the option's "...", \\16 an option opening the saints, \\10 the potions, \\06 a battle, # \\01 ... \\13 a part shown or hidden by the game. { is o-umlaut, \| u-umlaut, \\1f a-umlaut. # Put the edited file into mods/<your mod>/MSGFILES/ (keep the name); lines starting with # are ignored.  |
| `mods/datamods.c:397` | Describe your mod here (these lines show in F12 > Mods).  Put files into this folder laid out like the game folder; while the mod is on, the game reads them instead of its own: DARKLAND.LST, DARKLAND.ENM, PICS/NAME.PIC, a folder MSGFILES/ with single card files (.MSG, or .TXT as written by "Export the card texts"), art/ and sounds/ as in the game folder, music/ with recordings named by the piece's number (10.ogg, "11 market.ogg"; see F12 > Settings > Sound).  |
| `mods/encounters.c:276` | A new encounter stopped because of a mistake in its file (see darklands_port.log); the party is back in the overland view. |
| `mods/encounters.c:326` | '%s' expected |
| `mods/encounters.c:429` | %s takes %d argument%s |
| `mods/encounters.c:461` | %s: who, then an item (0 to 199) |
| `mods/encounters.c:469` | wounds: an attribute |
| `mods/encounters.c:476` | unknown function %s |
| `mods/encounters.c:495` | unknown name %s |
| `mods/encounters.c:498` | a value expected |
| `mods/encounters.c:504` | nested too deeply |
| `mods/encounters.c:562` | unexpected "%s" |
| `mods/encounters.c:576` | %s takes %d to %d values |
| `mods/encounters.c:587` | nested too deeply |
| `mods/encounters.c:600` | no section [%s] |
| `mods/encounters.c:627` | %s: %s expected |
| `mods/encounters.c:633` | an action expected |
| `mods/encounters.c:652` | 'then' or 'goto' expected |
| `mods/encounters.c:662` | card: one of the game's cards, 1 to 0x16d |
| `mods/encounters.c:675` | unknown variable %s |
| `mods/encounters.c:681` | picture names have at most 8 letters |
| `mods/encounters.c:690` | options are numbered 0 to 9 |
| `mods/encounters.c:696` | battle: a battlefield, then enemy, quality, count for each group |
| `mods/encounters.c:700` | battle: a wilderness battlefield (Wilds, Surrounded, Clearing, Cave, or 0x2e-0x55, 0x61-0x63, 0x6f-0x79) |
| `mods/encounters.c:725` | %s: who, then a %s |
| `mods/encounters.c:725` | skill (0 to 18) |
| `mods/encounters.c:725` | attribute (0 to 6) |
| `mods/encounters.c:747` | unknown action %s |
| `mods/encounters.c:751` | text %ld: %s has blocks 0 to %d |
| `mods/encounters.c:752` | text %ld: a card holds blocks 0 to 24 |
| `mods/encounters.c:798` | ';' expected |
| `mods/encounters.c:807` | unexpected "%s" |
| `mods/encounters.c:941` | unknown setting %s |
| `mods/encounters.c:947` | option: a number 0 to 9 |
| `mods/encounters.c:949` | option %d twice |
| `mods/encounters.c:952` | option %d: potion or saint |
| `mods/encounters.c:965` | at most 4 potions |
| `mods/encounters.c:967` | potion CODE, CHANCE |
| `mods/encounters.c:989` | unknown key %s |
| `mods/encounters.c:991` | no card (the MSGFILES text) given |
| `mods/encounters.c:992` | no picture given |
| `mods/encounters.c:993` | no [section] |
| `mods/encounters.c:1069` | %s has %d text blocks: the game holds 25 |
| `mods/encounters.c:1070` | %s is too long: %ld bytes as the game loads it, under 6000 fit |
| `mods/encounters.c:1071` | %s has a block with %d option lines: at most %d |
| `mods/encounters.c:1083` | [%s] has options but no block |
| `mods/encounters.c:1086` | [%s] option %d: block %d of %s has %d option line%s |
| `mods/encounters.c:1089` | [%s] option %d: %s |
| `mods/encounters.c:1089` | its line (\\16) opens the saints: write option N saint |
| `mods/encounters.c:1089` | a saint option needs a \\16 line |
| `mods/encounters.c:1090` | [%s] option %d: %s |
| `mods/encounters.c:1090` | its line (\\10) opens the potions: write option N potion |
| `mods/encounters.c:1090` | a potion option needs a \\10 line |
| `mods/encounters.c:1099` | [%s] option %d is a saint option, but there are no saints |
| `mods/encounters.c:1100` | [%s] option %d is a potion option, but there are no items |
| `mods/encounters.c:1102` | [%s] has a block but no options |
| `mods/encounters.c:1104` | [%s] saints: numbers 0 to 135 (DARKLAND.LST), or -1 |
| `mods/encounters.c:1105` | [%s] items: numbers 0 to 199 (DARKLAND.LST), or -1 |
| `mods/encounters.c:1107` | [%s] block %d: the game holds 25 blocks (0 to 24) |
| `mods/encounters.c:1154` | (Darklands+ %s) |
| `mods/encounters.c:1172` | New encounters: %d file%s with errors left out (see darklands_port.log). |
| `mods/fixgroups.c:10` | Crashes, freezes, items or progress lost, saved games spoiled, quests that can never be finished. Nobody wants these. |
| `mods/fixgroups.c:11` | Plain mistakes |
| `mods/fixgroups.c:11` | Something the game sets out to do and gets wrong: the wrong member, the wrong town, a check on the wrong option, a number lost on the way. |
| `mods/fixgroups.c:12` | As the manual and the game's texts say |
| `mods/fixgroups.c:12` | The game does something other than what its manual, or its own texts, say it does. Fixed to match the words. |
| `mods/fixgroups.c:13` | Old tricks closed |
| `mods/fixgroups.c:13` | Bugs players have long used to their advantage (invulnerability from fire protection, confession's clock turned back, the same bounty again and again). Leave these off to keep the old tricks. |
| `mods/fixgroups.c:14` | Left over from 1992 |
| `mods/fixgroups.c:14` | Not bugs, but nobody misses them: the question from the printed manual before the party first sets out. |
| `mods/gamemodel.c:258` | Divine Favor |
| `mods/gamemodel.c:263` | Edged Weapons |
| `mods/gamemodel.c:263` | Impact Weapons |
| `mods/gamemodel.c:263` | Thrown Weapons |
| `mods/gamemodel.c:263` | Missile Devices |
| `mods/gamemodel.c:264` | Speak Common |
| `mods/gamemodel.c:264` | Read & Write |
| `mods/gamemodel.c:283` | Missile Devices |
| `mods/hdart.c:779` | Create a New World |
| `mods/hdart.c:780` | The Story Continues |
| `mods/hdart.c:992` | %d pictures written to %s%s |
| `mods/hdart.c:992` |  (some could not be written) |
| `mods/hdtext.c:455` | cursor: shown %d page %d save %d,%d %dx%d at %d,%d |
| `mods/levelwalk.c:69` | The party cannot get there from where it stands. |
| `mods/levelwalk.c:79` | The party will walk there in single file (press G to move as a group again); Space resumes the battle. |
| `mods/levelwalk.c:80` | The party will walk there: Space resumes the battle. |
| `mods/levelwalk.c:81` | The party walks there in single file (press G to move as a group again). |
| `mods/levelwalk.c:141` | There are no stairs on this floor that the party has found. |
| `mods/levelwalk.c:144` | The party walks to the stairs and takes them. |
| `mods/lodgeview.c:140` | Castle Guard |
| `mods/lodgeview.c:143` | Castle Servant |
| `mods/lodgeview.c:144` | Castle Steward |
| `mods/lodgeview.c:144` | House Steward |
| `mods/lodgeview.c:145` | Preaching Friar |
| `mods/lodgeview.c:145` | Preaching Friar |
| `mods/lodgeview.c:145` | Street Preacher |
| `mods/lodgeview.c:146` | Guard Sergeant |
| `mods/lodgeview.c:146` | Mounted Sergeant |
| `mods/lodgeview.c:149` | Street Performer |
| `mods/lodgeview.c:150` | Mendicant Friar |
| `mods/lodgeview.c:150` | Hospital Helper |
| `mods/lodgeview.c:151` | Town Militia |
| `mods/lodgeview.c:467` | The days stop: %s's teacher has gone. |
| `mods/lodgeview.c:469` | The days stop: what %s does each day has changed. |
| `mods/lodgeview.c:471` | The days stop: %s has regained all Strength. |
| `mods/lodgeview.c:472` | The days stop: %s's Divine Favor is full. |
| `mods/looks.c:85` | Gray-haired fighter |
| `mods/looks.c:85` | Brown-haired woman |
| `mods/looks.c:85` | Blond fighter |
| `mods/looks.c:87` | Knight in plate |
| `mods/looks.c:622` | Stout priest |
| `mods/looks.c:821` | Sprite %d of %d: %s (an enemy's sprite, added by Darklands+%s) |
| `mods/looks.c:822` | ; the 1st, 2nd and 3rd color buttons recolor it |
| `mods/looks.c:822` | ; the 1st and 2nd color buttons recolor it |
| `mods/looks.c:823` | ; the 1st color button recolors it |
| `mods/looks.c:824` | Sprite %d of %d: %s |
| `mods/looks.c:839` | %s color: %s |
| `mods/looks.c:842` | This sprite has no more colors that can be changed. |
| `mods/looks.c:842` | This sprite's colors are fixed: they cannot be changed. |
| `mods/looks.c:868` | PT BW CB HG TA TK DT |
| `mods/loot.c:105` | Nobody has room: every member already carries 64 kinds of items. |
| `mods/loot.c:106` | No room: this member already carries 64 kinds of items (S shares the pile out among the party). |
| `mods/loot.c:107` | %s %d; %d left in the pile (no room for more kinds of items). |
| `mods/loot.c:108` | Shared out all %d. |
| `mods/loot.c:108` | Took all %d. |
| `mods/loot.c:217` | Thrown weapons picked up after the battle (Darklands+):  |
| `mods/loot.c:280` | Point at one of the merchant's goods first (the upper list). |
| `mods/loot.c:288` | Bought none: not enough money, or no room. |
| `mods/loot.c:290` | Point at one of your own items first (the lower list). |
| `mods/loot.c:306` | This merchant does not buy that. |
| `mods/lootview.c:149` | %d %s to %s (to give it to another member, drag it from the character sheet onto them in the party panel). |
| `mods/lootview.c:150` | %d %s into the party's pack. |
| `mods/lootview.c:151` | No room: every member already carries 64 kinds of items. |
| `mods/lootview.c:153` | %s takes %d %s. |
| `mods/lootview.c:154` | %s has no room for another kind of item (64 at most). |
| `mods/lootview.c:164` | The %s is marked Keep on the character sheet: it stays in the party's pack. |
| `mods/lootview.c:178` | %d %s back into the pile. |
| `mods/lootview.c:179` | The %s stays in the party's pack. |
| `mods/lootview.c:202` | %s puts %d %s into the pile. |
| `mods/lootview.c:203` | The %s stays with %s. |
| `mods/lootview.c:280` | Took what fits. |
| `mods/lootview.c:280` | Took everything. |
| `mods/lootview.c:284` | Took what fits. |
| `mods/lootview.c:284` | Took everything. |
| `mods/lootview.c:285` | Shared out what fits. |
| `mods/lootview.c:285` | Shared out everything. |
| `mods/losses.c:96` | Lost for good |
| `mods/losses.c:96` | The maximum of an attribute, or a skill, is lower now, and will not come back by itself. |
| `mods/losses.c:119` | worn to nothing and gone (it was %dq). |
| `mods/losses.c:120` | now %dq (was %dq). |
| `mods/losses.c:127` | Armor worn down |
| `mods/losses.c:127` | Acid and fire wear armor down for good: a smith's work cannot mend it; a better piece must be bought. |
| `mods/menuview.c:169` | Darklands+ window |
| `mods/menuview.c:169` | Quests (world map) |
| `mods/menuview.c:171` | New Player Guide |
| `mods/menuview.c:171` | Keys and clicks on this screen |
| `mods/musicpack.c:123` | the inn (one of the two at random) |
| `mods/musicpack.c:125` | praying to a saint |
| `mods/musicpack.c:126` | the guilds and craftsmen |
| `mods/musicpack.c:127` | the city square |
| `mods/musicpack.c:128` | the university |
| `mods/musicpack.c:129` | the shell game |
| `mods/musicpack.c:130` | the fortress |
| `mods/musicpack.c:131` | the party's end |
| `mods/musicpack.c:132` | the overland view (one of two at random) |
| `mods/musicpack.c:133` | the overland view (one of two at random); making the party |
| `mods/musicpack.c:135` | a grove in town by day |
| `mods/musicpack.c:136` | the dragon's lair |
| `mods/musicpack.c:137` | before a city by day (four tunes in turn) |
| `mods/musicpack.c:138` | before a city by night |
| `mods/musicpack.c:139` | a grove in town by night |
| `mods/musicpack.c:140` | an ambush, or something hidden ahead |
| `mods/musicpack.c:141` | the wild hunt |
| `mods/musicpack.c:142` | the woodwife |
| `mods/musicpack.c:144` | a dream in camp |
| `mods/musicpack.c:145` | the manor by the misty lake |
| `mods/musicpack.c:147` | the Templars |
| `mods/musicpack.c:151` | a short sound played over the current piece |
| `mods/musicpack.c:285` | RLND DrkLand03-16-93 |
| `mods/offices.c:60` | You finished a quest for them: your reward is waiting. |
| `mods/offices.c:61` | You are on a quest for them. They offer no other until it is done. |
| `mods/offices.c:65` | You asked lately, or were paid a reward: no new quest for three days. |
| `mods/offices.c:67` | The Hanse will not deal with you: your reputation in this town is too low. |
| `mods/offices.c:68` | No quests while your reputation in this town, or your standing with the bank, is below zero. |
| `mods/offices.c:73` | Each time you ask (by day), there is %s %d%% chance of a quest: %s. If none comes, or you turn it down, there is no new quest for three days. |
| `mods/offices.c:74` | 3 in 10 are robber knights to hunt, the rest items to fetch |
| `mods/offices.c:74` | a robber knight to hunt or an item to fetch, half and half |
| `mods/offices.c:77` | This cathedral keeps no relic, so its canon has no quests. |
| `mods/offices.c:78` | You asked lately: the canon will offer no quest for three days. |
| `mods/offices.c:81` | Each time you ask the canon (by day), there is %s %d%% chance of a quest (your reputation in this town minus 50): a robber knight to hunt or an item to fetch. If none comes, or you turn it down, there is no new quest for three days. |
| `mods/offices.c:84` | You asked for an audience lately: for a week the chance is only 1%. |
| `mods/offices.c:85` | The lord grants no audience while your reputation in this town is -40 or lower. |
| `mods/offices.c:88` | You finished a quest for the lord: your reward is waiting (an audience is certain). |
| `mods/offices.c:90` | You finished a quest for the lord. In a town without a council the original game never pays this reward; with the fix "The lord pays for his quests" on, he pays it at an audience. |
| `mods/offices.c:91` | Each request for an audience with the council has %s %d%% chance of success (it depends on your reputation in this town, and the speaker's Charisma and Speak Common). An audience brings a robber knight to hunt or an item to fetch, half and half. After a request the chance is 1%% for a week. |
| `mods/offices.c:92` | Each request for an audience with the lord has %s %d%% chance of success (it depends on your reputation in this town, and the speaker's Charisma and Speak Common). An audience brings a robber knight to hunt. After a request the chance is 1%% for a week. |
| `mods/offices.c:95` | No quests for three days: at your last visit none came, one was turned down, or a reward was paid. |
| `mods/offices.c:97` | The offer comes after shopping: as the party leaves the stalls, about one time in seven a trader takes it aside with a purse and a job (mostly an item to fetch, sometimes a robber knight to hunt). If none comes, there is no other offer for three days. |
| `mods/orders.c:609` | Click: throw here |
| `mods/orders.c:609` | Click: flee here |
| `mods/orders.c:610` | No clear throw |
| `mods/orders.c:610` | Can't walk there |
| `mods/orders.c:611` | No clear shot |
| `mods/orders.c:611` | No clear throw |
| `mods/orders.c:611` | No way there |
| `mods/orders.c:618` | Click: shoot (moves for a clear line) |
| `mods/orders.c:619` | Click: shoot |
| `mods/orders.c:621` | Click: throw |
| `mods/orders.c:622` | Click: attack (Vulnerable) |
| `mods/orders.c:623` | Click: attack (Berserk) |
| `mods/orders.c:624` | Click: parry its blows |
| `mods/orders.c:628` | No clear shot: walks up to fight |
| `mods/pathfix.c:448` |  (stuck: around everyone) |
| `mods/pathfix.c:628` | cells unreachable |
| `mods/pathfix.c:628` | search gave up |
| `mods/pathfix.c:633` | over 20 cells |
| `mods/pathfix.c:637` | time native %u us, original %u us; where both: %ld vs %ld steps  |
| `mods/politics.c:205` | While you were away, the rebels marched on the %s: the city has a new government. |
| `mods/politics.c:206` | While you were away, the rebels marched on the %s and were scattered: the old government holds. |
| `mods/politics.c:207` | While you were away, the city guard arrested the rebel leaders: the old government holds. |
| `mods/politics.c:208` | While you were away, the guard tried to arrest the rebel leaders and the crowd freed them: the city has a new government. |
| `mods/politics.c:270` | are undecided about |
| `mods/politics.c:310` | $CityLord Title |
| `mods/politics.c:347` | the exiled heir |
| `mods/politics.c:389` | there are some interesting rumours flying around |
| `mods/politics.c:468` | Plans in place |
| `mods/politics.c:522` | the rebellion was put down |
| `mods/rosterview.c:97` | allow ... to retire |
| `mods/rosterview.c:196` | %s joined the party at another inn, so cannot be left at this one. To part with them: Done, then "allow %s to retire". |
| `mods/rosterview.c:201` | Add someone to the party first. |
| `mods/rosterview.c:202` | The party is full: up to four travel together. Drop someone first. |
| `mods/rosterview.c:216` | The new character is in the list: add them to the party. |
| `mods/saintinfo.c:27` | Endurance +8 to 15, Charisma +6 to 11, every weapon skill +15 to 29. |
| `mods/saintinfo.c:29` | Heals 8 to 15 Endurance (up to its maximum). Charisma +6 to 11, every weapon skill +15 to 29. |
| `mods/saintinfo.c:30` | Endurance +6 to 11 (a woman: +12 to 23), Charisma +6 to 11. |
| `mods/saintinfo.c:32` | Heals 6 to 11 Endurance (a woman: 12 to 23; up to its maximum). Charisma +6 to 11. |
| `mods/saintinfo.c:33` | Only if a woman is in the party: may raise the reputation in the nearest town by 10 to 19; Charisma +10 to 29. |
| `mods/saintinfo.c:36` | Woodwise +25 to 29. |
| `mods/saintinfo.c:39` | Intelligence +8 to 15, Perception +6 to 11, Alchemy +30 to 44, Latin and Read & Write +15 to 29. |
| `mods/saintinfo.c:42` | Intelligence +6 to 11, Latin and Read & Write +20 to 39. |
| `mods/saintinfo.c:45` | May raise the reputation in the nearest town by 10 to 19. Charisma +6 to 11. A party with neither florins nor groschen gets 3 to 9 groschen. |
| `mods/saintinfo.c:48` | Endurance +6 to 11, Charisma +8 to 15. |
| `mods/saintinfo.c:49` | No reputation in Scotland or Russia, which are not in the game. |
| `mods/saintinfo.c:51` | Edged, Impact, Polearms, Thrown and Bows +15 to 29. |
| `mods/saintinfo.c:54` | Endurance +4 to 9, Strength +5 to 10, Charisma +1 to 10, Speak Common and Latin +10 to 19, Read & Write +20 to 39. |
| `mods/saintinfo.c:56` | Heals 4 to 9 Endurance (up to its maximum). Strength +5 to 10, Charisma +1 to 10, Speak Common and Latin +10 to 19, Read & Write +20 to 39. |
| `mods/saintinfo.c:57` | Perception +6 to 11, Strength +4 to 7, Charisma +15 to 24, Speak Common +20 to 29, Impact and Latin +10 to 19. |
| `mods/saintinfo.c:60` | Perception +6 to 11, Strength and Intelligence +4 to 7, every weapon skill +10 to 19, Streetwise and Latin +6 to 11, Read & Write and Riding +8 to 15. |
| `mods/saintinfo.c:63` | Endurance +8 to 23, Strength +2 to 4. |
| `mods/saintinfo.c:65` | Heals 30% of lost Endurance and 10% of lost Strength. |
| `mods/saintinfo.c:66` | Artifice +15 to 29 for two hours at most; Missile Devices +20 to 39 for 5 to 7 hours. |
| `mods/saintinfo.c:68` | Artifice +15 to 29 for two hours at most; Missile Devices +20 to 39 for 5 to 7 hours. In a battle, wounds cannot bring the one prayed for below 1 Strength: no death from wounds. |
| `mods/saintinfo.c:69` | Intelligence +4 to 8, Perception +3 to 6. |
| `mods/saintinfo.c:72` | Charisma +8 to 15, every weapon skill +10 to 19, Speak Common +8 to 15. |
| `mods/saintinfo.c:75` | Charisma and Intelligence +8 to 15, Speak Common +40 to 99. |
| `mods/saintinfo.c:78` | Charisma and Perception +1 to 15, Speak Common +10 to 29. |
| `mods/saintinfo.c:81` | Charisma +3 to 10. @INSTR, the reputation of the town the party is in (in the overland view: the nearest) may rise by 20 to 59. |
| `mods/saintinfo.c:84` | Perception and Intelligence +3 to 5, Strength +6 to 11, every weapon skill and Latin +6 to 11. |
| `mods/saintinfo.c:87` | @FIRE1 May raise the reputation in the nearest town by 10 to 19. Charisma +6 to 11, Flails +15 to 29. |
| `mods/saintinfo.c:88` | Flails and the reputation are not in the text. The fire protection is only for the one prayed for (the text promises it to everyone, and much more to one person), and no Endurance is healed. |
| `mods/saintinfo.c:89` | Fire protection 50 for every member, 90 for the one prayed for. Heals 10% of lost Endurance. |
| `mods/saintinfo.c:90` | Agility +8 to 15, Charisma +1 to 10. |
| `mods/saintinfo.c:93` | Streetwise +10 to 19, Woodwise +20 to 39, Riding +25 to 74, Bows +6 to 11. |
| `mods/saintinfo.c:94` | No faster travel, and no Strength or Endurance restored. |
| `mods/saintinfo.c:95` | The party travels half again as fast for a week (fewer days on the road). Heals 10% of lost Strength and 20% of lost Endurance. Streetwise +10 to 19, Woodwise +20 to 39, Riding +25 to 74, Bows +6 to 11. |
| `mods/saintinfo.c:96` | Heals all lost Endurance and Strength. |
| `mods/saintinfo.c:99` | Heals 25% of lost Endurance and 50% of lost Strength. Charisma +6 to 11, Healing +10 to 19. |
| `mods/saintinfo.c:102` | Charisma +8 to 15, Speak Common +30 to 89, Latin +6 to 11, Read & Write +10 to 19. |
| `mods/saintinfo.c:105` | Endurance +8 to 15, Strength +6 to 13, Charisma +4 to 7, Perception lowered by 5 to 9, the skills of the weapons in hand +10 to 19. |
| `mods/saintinfo.c:108` | Heals 10% of lost Endurance and 10% of lost Strength. Perception +4 to 7, Healing +40 to 79. Teaches St. Damian. |
| `mods/saintinfo.c:112` | The text says non-metal armor; the game raises all armor. |
| `mods/saintinfo.c:113` | Every member's leather and cloth armor counts 10 points better in quality. |
| `mods/saintinfo.c:114` | Intelligence +5 to 9, Alchemy +5 to 9. |
| `mods/saintinfo.c:117` | Heals all lost Endurance and 30% of lost Strength. Teaches St. Cosmas. |
| `mods/saintinfo.c:120` | Intelligence +4 to 7, Charisma +1 to 6, Perception +6 to 11, the skills of the weapons in hand +8 to 15, Riding +6 to 11. |
| `mods/saintinfo.c:123` | Heals all lost Strength. Charisma +15 to 44, Speak Common +25 to 49. |
| `mods/saintinfo.c:124` | No reputation in Paris and France, which are not in the game. |
| `mods/saintinfo.c:125` | Heals all lost Strength and 10% of lost Endurance. Charisma +15 to 44, Speak Common +25 to 49. |
| `mods/saintinfo.c:125` | No reputation in Paris and France, which are not in the game. |
| `mods/saintinfo.c:126` | Heals 30% of lost Endurance. Strength +5 to 14, the skills of the weapons in hand +20 to 59. |
| `mods/saintinfo.c:129` | Heals all lost Endurance and 20% of lost Strength. Charisma +15 to 29. |
| `mods/saintinfo.c:130` | No reputation in Corsica and Monaco, which are not in the game. |
| `mods/saintinfo.c:132` | Agility +10 to 19, Woodwise and Stealth +10 to 19, Streetwise and Artifice +20 to 39. |
| `mods/saintinfo.c:133` | Only the one prayed for, not the entire party; no Charisma or Speak Common. |
| `mods/saintinfo.c:134` | Every member's Stealth +10 to 19. Agility, Woodwise +10 to 19, Streetwise and Artifice +20 to 39, Charisma and Speak Common +6 to 11. |
| `mods/saintinfo.c:135` | Intelligence +10 to 19, Speak Common and Latin +10 to 19, Read & Write +20 to 39. |
| `mods/saintinfo.c:138` | Charisma +10 to 19, Perception +6 to 11, Healing +10 to 19. A member with no edged weapon gets a long sword. |
| `mods/saintinfo.c:141` | Perception +6 to 11, Healing +10 to 19, Woodwise +20 to 39. For a week no ambush takes the party by surprise (bandits and the like). |
| `mods/saintinfo.c:142` | No faster travel; instead, a week safe from ambushes. |
| `mods/saintinfo.c:143` | The party travels half again as fast for a week. Perception +6 to 11, Healing +10 to 19, Woodwise +20 to 39. |
| `mods/saintinfo.c:144` | Perception +6 to 11, the skills of the weapons in hand +6 to 11, Artifice +25 to 74, Streetwise +6 to 11, Alchemy +5 to 14. @ARM1 |
| `mods/saintinfo.c:145` | The text says the quality of all metal armor in the party; the game gives the one prayed for armor one level thicker. |
| `mods/saintinfo.c:146` | Every member's metal armor (scale, mail, plate) counts 10 points better in quality. Perception +6 to 11, the skills of the weapons in hand +6 to 11, Artifice +25 to 74, Streetwise +6 to 11, Alchemy +5 to 14. |
| `mods/saintinfo.c:147` | Heals 30% of lost Endurance. Agility +4 to 7, Stealth +15 to 29, Healing +10 to 19. |
| `mods/saintinfo.c:150` | Endurance +4 to 7, Intelligence +6 to 11, Perception +8 to 15, Riding +8 to 15, every weapon skill +6 to 11. Every member's Virtue is raised to at least 20, for good, and the party loses a quarter of its money. |
| `mods/saintinfo.c:151` | Endurance is raised past its maximum for a day, not healed; a rich party loses less than a quarter (the game counts only part of the purse). |
| `mods/saintinfo.c:152` | Heals 4 to 7 Endurance (up to its maximum). Intelligence +6 to 11, Perception +8 to 15, Riding +8 to 15, every weapon skill +6 to 11. Every member's Virtue is raised to at least 20, for good, and the party loses a quarter of its money. |
| `mods/saintinfo.c:153` | Artifice +25 to 74, Alchemy +8 to 15. |
| `mods/saintinfo.c:155` | Blades, hammers and maces, polearms, crossbows and guns of the one prayed for count 10 points better in quality. Artifice +25 to 74, Alchemy +8 to 15. |
| `mods/saintinfo.c:156` | Charisma +15 to 29, Speak Common +20 to 39. |
| `mods/saintinfo.c:159` | Charisma +5 to 9, Riding and Woodwise +10 to 19, Speak Common +6 to 11. The party can cross water as if it were land. |
| `mods/saintinfo.c:162` | @FIRE1 Agility and Perception +6 to 11. |
| `mods/saintinfo.c:164` | Fire protection 50 for every member. Agility and Perception +6 to 11. |
| `mods/saintinfo.c:165` | Strength +8 to 23, Charisma +4 to 7, the skills of the weapons in hand +15 to 24, Riding and Speak Common +6 to 11. |
| `mods/saintinfo.c:166` | No reputation in Sweden, which is not in the game. |
| `mods/saintinfo.c:168` | Riding +10 to 19, Woodwise +40 to 69, Edged, Thrown and Bows +6 to 11. |
| `mods/saintinfo.c:169` | No faster travel through woods, and the weapon skills are only Edged, Thrown and Bows. |
| `mods/saintinfo.c:170` | The party travels twice as fast through woods and forests for a week. Riding +10 to 19, Woodwise +40 to 69, Edged, Polearms, Thrown, Bows and Missile Devices +15 to 29. |
| `mods/saintinfo.c:171` | Agility +8 to 15, Stealth +40 to 119, Woodwise +10 to 19. |
| `mods/saintinfo.c:174` | Charisma +8 to 15, Perception +6 to 11, Latin +10 to 19. |
| `mods/saintinfo.c:177` | Agility +8 to 15, Charisma +6 to 11, Streetwise +4 to 7, Speak Common and Read & Write +6 to 11. The party can cross water as if it were land. |
| `mods/saintinfo.c:180` | May raise the reputation by 15 to 24 in Passau, Salzburg, Linz, Steyr, Wien or Graz (nowhere else). The skills of the weapons in hand +6 to 11, Riding +6 to 11. The party can cross water as if it were land. |
| `mods/saintinfo.c:182` | May raise the reputation by 15 to 24 in Passau, Salzburg, Linz, Steyr, Wien, Graz, Posen or Bromberg (nowhere else). The skills of the weapons in hand +6 to 11, Riding +6 to 11. The party can cross water as if it were land. |
| `mods/saintinfo.c:183` | Heals 30% of lost Endurance and 10% of lost Strength. Perception and Endurance +6 to 11, Charisma +15 to 29, Speak Common and Woodwise +4 to 7, Healing +10 to 19, Artifice +12 to 23. |
| `mods/saintinfo.c:184` | No Virtue and no loss of money; instead Healing, Artifice and Woodwise. |
| `mods/saintinfo.c:185` | Endurance and Perception +6 to 11, Charisma +15 to 29, Speak Common +10 to 19, Virtue +25 to 49. One time in two the party loses a tenth of its money. |
| `mods/saintinfo.c:186` | Intelligence and Perception +10 to 29. |
| `mods/saintinfo.c:189` | Heals 30% of lost Endurance and 20% of lost Strength. For a week no ambush takes the party by surprise (bandits and the like). |
| `mods/saintinfo.c:192` | Strength +10 to 14, Agility +5 to 9, every weapon skill and Riding +25 to 49. |
| `mods/saintinfo.c:194` | The weapon and the armor of the one prayed for count 10 points better in quality. Strength +10 to 14, Agility +5 to 9, every weapon skill and Riding +25 to 49. |
| `mods/saintinfo.c:195` | Heals 30% of lost Endurance and 20% of lost Strength. The skills of the weapons in hand +15 to 24, Healing +20 to 39. |
| `mods/saintinfo.c:196` | No Charisma, and only the skills of the weapons in hand, not all weapon skills. |
| `mods/saintinfo.c:197` | Heals 30% of lost Endurance and 20% of lost Strength. Every weapon skill +15 to 24, Healing +20 to 39, Charisma +6 to 11. |
| `mods/saintinfo.c:198` | Charisma +6 to 11, Riding +5 to 9, Woodwise, Latin and Read & Write +15 to 29. At the party's next step in the overland view it is carried to the nearest town. |
| `mods/saintinfo.c:201` | Heals 40% of lost Endurance and 10% of lost Strength. Agility lowered by 4 to 7, the skills of the weapons in hand lowered by 15 to 24, Stealth +30 to 59, Woodwise +5 to 9, Streetwise +50 to 89. |
| `mods/saintinfo.c:202` | Woodwise and Streetwise the other way around (the text: a little Streetwise, a tremendous Woodwise); no money. |
| `mods/saintinfo.c:203` | Heals 40% of lost Endurance and 10% of lost Strength. Agility lowered by 4 to 7, the skills of the weapons in hand lowered by 15 to 24, Stealth +30 to 59, Woodwise +50 to 89, Streetwise +5 to 9. A party with neither florins nor groschen gets 3 to 9 groschen. |
| `mods/saintinfo.c:204` | Intelligence +5 to 9, Perception +15 to 24, Alchemy +40 to 59, Artifice +10 to 19. Costs a point of Strength (a wound) and a point of Virtue, for good. |
| `mods/saintinfo.c:207` | Heals 30% of lost Endurance. Latin +20 to 39, Read & Write +25 to 49. |
| `mods/saintinfo.c:210` | Heals 20% of lost Endurance. Speak Common +15 to 29, Artifice +10 to 19. For a week no ambush takes the party by surprise (bandits and the like). |
| `mods/saintinfo.c:213` | May raise the reputation by 20 to 39 in Schleswig, Lübeck, Wismar, Rostock, Hamburg, Lüneburg or Bremen (nowhere else). Every weapon skill +15 to 29. |
| `mods/saintinfo.c:216` | Charisma +8 to 15, Alchemy and Speak Common +10 to 19, Artifice +20 to 39. |
| `mods/saintinfo.c:219` | May raise the reputation by 30 to 49 in Frankfurt an der Oder, Breslau, Olmütz or Teschen (nowhere else). Healing +20 to 29. |
| `mods/saintinfo.c:221` | May raise the reputation by 30 to 49 in Frankfurt an der Oder, Breslau, Olmütz or Teschen (nowhere else). Heals 20% of lost Endurance and Strength. Healing +20 to 29. |
| `mods/saintinfo.c:222` | Intelligence +8 to 23, every weapon skill +10 to 19. |
| `mods/saintinfo.c:223` | No reputation in its help. |
| `mods/saintinfo.c:225` | Charisma +10 to 19, Strength +4 to 7. |
| `mods/saintinfo.c:228` | Perception +15 to 29. |
| `mods/saintinfo.c:231` | Perception +20 to 59. |
| `mods/saintinfo.c:232` | No reputation in its help. |
| `mods/saintinfo.c:233` | Perception +20 to 59. By the Rhine (Basel to Xanten), may raise the reputation by 5 to 14. |
| `mods/saintinfo.c:234` | Polearms, Thrown and Bows +15 to 29, Missile Devices +10 to 19, Stealth +25 to 49, Woodwise +40 to 79. |
| `mods/saintinfo.c:237` | Charisma +15 to 29, every weapon skill and Speak Common +10 to 19, Riding +5 to 9. |
| `mods/saintinfo.c:240` | Charisma +8 to 15, Streetwise +10 to 19, Speak Common +5 to 9. |
| `mods/saintinfo.c:241` | No reputation in Madrid and Spain, which are not in the game. |
| `mods/saintinfo.c:243` | Heals 50% of lost Endurance and all lost Strength. Charisma +8 to 15, Healing +20 to 39. |
| `mods/saintinfo.c:246` | Heals 20% of lost Endurance and 10% of lost Strength. Strength and Endurance +4 to 7, Charisma +3 to 5, Virtue +20 to 34. |
| `mods/saintinfo.c:247` | No reputation in Spain, which is not in the game; no Intelligence lost. |
| `mods/saintinfo.c:248` | Heals 20% of lost Endurance and 10% of lost Strength. Strength and Endurance +4 to 7, Charisma +3 to 5, Virtue +20 to 34, Intelligence lowered by 3 to 5. |
| `mods/saintinfo.c:248` | No reputation in Spain, which is not in the game. |
| `mods/saintinfo.c:249` | No help of its own: its powers work only where an encounter calls for it. |
| `mods/saintinfo.c:252` | A woman: heals all lost Endurance and 20% of lost Strength, and Endurance +5 to 9, Strength +4 to 7. A man: heals 30% of lost Endurance and 10% of lost Strength. |
| `mods/saintinfo.c:255` | Charisma +8 to 15, Perception lowered by 6 to 11, Speak Common and Latin +30 to 59. |
| `mods/saintinfo.c:258` | Alchemy, Religion, Speak Common, Latin, Read & Write, Healing, Artifice, Stealth, Streetwise and Woodwise each +1 to 4, and 1 more for every 5 points of Virtue. |
| `mods/saintinfo.c:259` | Not all skills: no weapon skills and no Riding. |
| `mods/saintinfo.c:260` | Every skill +1 to 4, and 1 more for every 5 points of Virtue. |
| `mods/saintinfo.c:261` | @NEPOMUK Charisma +8 to 15, Intelligence +6 to 11, Speak Common +20 to 39, Streetwise +4 to 7. |
| `mods/saintinfo.c:264` | Charisma +6 to 11, Artifice and Streetwise +30 to 59. |
| `mods/saintinfo.c:266` | Artifice and Streetwise +30 to 59; in a town, Charisma +6 to 11. |
| `mods/saintinfo.c:267` | Endurance, Charisma, Strength, Perception, Agility and Intelligence +4 to 8, the skills of the weapons in hand +15 to 24, Alchemy, Religion, Speak Common, Latin, Read & Write, Healing, Stealth, Artifice, Woodwise and Streetwise +6 to 11. |
| `mods/saintinfo.c:268` | Not every skill: no Riding, and no weapon skill but those of the weapons in hand. |
| `mods/saintinfo.c:269` | Endurance, Charisma, Strength, Perception, Agility and Intelligence +4 to 8, the skills of the weapons in hand +15 to 24, every other skill +6 to 11. |
| `mods/saintinfo.c:270` | Woodwise +20 to 39, Riding +6 to 11. The party can cross water as if it were land. |
| `mods/saintinfo.c:273` | Heals 40% of lost Endurance and 20% of lost Strength. Charisma +6 to 11, Speak Common +15 to 29, Riding +4 to 7, Healing +10 to 19. |
| `mods/saintinfo.c:276` | Strength +4 to 7, Agility +3 to 5, the skills of the weapons in hand +15 to 24, Speak Common +6 to 11, Riding +10 to 19. |
| `mods/saintinfo.c:279` | Heals all lost Endurance. Charisma +12 to 19, Speak Common and Streetwise +10 to 19. |
| `mods/saintinfo.c:282` | Heals 30% of lost Endurance and all lost Strength. |
| `mods/saintinfo.c:285` | Every weapon skill +10 to 19. |
| `mods/saintinfo.c:287` | The weapon of the one prayed for counts 10 points better in quality. Every weapon skill +10 to 19. |
| `mods/saintinfo.c:288` | Heals all lost Endurance and Strength. Perception +6 to 11. |
| `mods/saintinfo.c:291` | Heals 50% of lost Endurance and 30% of lost Strength. Healing +20 to 39. |
| `mods/saintinfo.c:294` | Charisma +6 to 11, Perception +8 to 15, Endurance +5 to 9, Virtue +6 to 17. |
| `mods/saintinfo.c:295` | Endurance is raised, not spent. |
| `mods/saintinfo.c:296` | Costs 5 to 9 Endurance (not below 1). Charisma +6 to 11, Perception +8 to 15, Virtue +6 to 17. |
| `mods/saintinfo.c:297` | A woman: heals all lost Endurance and half the lost Strength, and every attribute +3 to 6 and Alchemy, Religion, Speak Common, Latin, Read & Write, Healing, Artifice, Stealth, Streetwise and Woodwise +5 to 10. A man: heals 10% of lost Endurance and Strength. |
| `mods/saintinfo.c:298` | No protection from flames. |
| `mods/saintinfo.c:299` | Fire protection 90 for the one prayed for. A woman: heals all lost Endurance and half the lost Strength, and every attribute +3 to 6 and Alchemy, Religion, Speak Common, Latin, Read & Write, Healing, Artifice, Stealth, Streetwise and Woodwise +5 to 10. A man: heals 10% of lost Endurance and Strength. |
| `mods/saintinfo.c:300` | Heals all lost Endurance and 40% of lost Strength. Healing +20 to 39, Speak Common +10 to 19. |
| `mods/saintinfo.c:303` | Agility +12 to 23, Read & Write +15 to 29. |
| `mods/saintinfo.c:306` | Heals 30% of lost Endurance and 30% of lost Strength. Charisma +8 to 15, Perception +12 to 19, Speak Common +25 to 49, Riding +20 to 39. |
| `mods/saintinfo.c:308` | One who wears armor has 30% of lost Endurance and Strength healed. Charisma +8 to 15, Perception +12 to 19, Speak Common +25 to 49, Riding +20 to 39. |
| `mods/saintinfo.c:309` | Intelligence +8 to 15, Latin +15 to 29, Read & Write +25 to 49. The party's standing with the Fugger and with the Medici rises by 20 to 39 each. |
| `mods/saintinfo.c:312` | Edged +20 to 39, Alchemy +10 to 19. |
| `mods/saintinfo.c:314` | An edged weapon of the one prayed for counts 10 points better in quality. Edged +20 to 39, Alchemy +10 to 19. |
| `mods/saintinfo.c:315` | May raise the reputation in the nearest town by 20 to 39. Charisma +12 to 19, the skills of the weapons in hand +20, Healing +20 to 39. |
| `mods/saintinfo.c:316` | In combat it raises the skills of the weapons in hand, not the quality of the weapons and armor. |
| `mods/saintinfo.c:317` | Every member's weapon and armor count 10 points better in quality. May raise the reputation in the nearest town by 20 to 39. Charisma +12 to 19, the skills of the weapons in hand +20, Healing +20 to 39. |
| `mods/saintinfo.c:318` | Charisma +10 to 19, Healing +20 to 39. The party can cross water as if it were land. |
| `mods/saintinfo.c:321` | Every weapon skill +15 to 29, Stealth +20 to 39, Woodwise and Streetwise +10 to 19. |
| `mods/saintinfo.c:324` | May raise the reputation in the nearest town by 10 to 19. Charisma +6 to 11, Endurance +4 to 8, Flails +15 to 29. |
| `mods/saintinfo.c:325` | Flails and Endurance instead of Agility and Riding; the reputation anywhere, not only in his lands. |
| `mods/saintinfo.c:326` | May raise the reputation in Nancy (Lorraine) by 10 to 19. Agility and Charisma +6 to 11, Riding +40 to 79. |
| `mods/saintinfo.c:326` | No reputation in Russia, Greece, Sicily and Apulia, which are not in the game. |
| `mods/saintinfo.c:327` | May raise the reputation by 15 to 24 in Strassburg or Basel (nowhere else). Perception +6 to 11. |
| `mods/saintinfo.c:330` | Perception +10 to 19, Charisma +4 to 8, every weapon skill +8 to 22, Speak Common +8 to 15. |
| `mods/saintinfo.c:333` | Heals 30% of lost Endurance. Every weapon skill +8 to 22. |
| `mods/saintinfo.c:336` | @FIRE1 Heals 30% of lost Endurance and 30% of lost Strength. Charisma +6 to 11, Alchemy +10 to 19, Healing +15 to 29. The party can cross water as if it were land. |
| `mods/saintinfo.c:337` | The fire protection is only for the one prayed for, not the party; the crossing of water is not in the text; nothing against animals. |
| `mods/saintinfo.c:338` | Fire protection 50 for every member. One who wears armor has 30% of lost Endurance and Strength healed. Alchemy +10 to 19, Healing +15 to 29. |
| `mods/saintinfo.c:338` | Nothing against animals. |
| `mods/saintinfo.c:339` | Charisma +15 to 24, Strength +12 to 19, Edged, Impact, Polearms, Speak Common, Latin and Read & Write +15 to 29. @ARM2 |
| `mods/saintinfo.c:340` | The armor is against every blow, not only missiles. |
| `mods/saintinfo.c:341` | Armor two levels thicker against missiles. Charisma +15 to 24, Strength +12 to 19, Edged, Impact, Polearms, Speak Common, Latin and Read & Write +15 to 29. |
| `mods/saintinfo.c:343` | No reputation in Greece and Malta, which are not in the game. |
| `mods/saintinfo.c:348` | Charisma +15 to 29. |
| `mods/saintinfo.c:351` | Charisma +8 to 15, Strength +12 to 19, Perception lowered by half its maximum, Speak Common, Latin and Healing +10 to 19, Edged +15 to 29. |
| `mods/saintinfo.c:354` | May lower the reputation in the nearest town by 1 to 4. Intelligence and Perception +6 to 11, Alchemy and Riding +10 to 19, Stealth +40 to 79. |
| `mods/saintinfo.c:360` | Heals all lost Endurance and Strength. Agility +8 to 15, Perception +6 to 11, Healing +50 to 139. |
| `mods/saintinfo.c:363` | Intelligence +15 to 29, Read & Write +30 to 89. The party can cross water as if it were land. |
| `mods/saintinfo.c:364` | No reputation with the Dominicans in its help. |
| `mods/saintinfo.c:366` | Intelligence +12 to 19, Alchemy +20 to 39, Read & Write +30 to 59. Endurance and Strength below 9 are raised to 9. |
| `mods/saintinfo.c:369` | Heals 30% of lost Endurance. Charisma lowered by 6 to 11, Alchemy +6 to 17, Artifice +35 to 69. |
| `mods/saintinfo.c:372` | Heals 20% of lost Endurance and 20% of lost Strength. Healing +10 to 19, Artifice +8 to 15. |
| `mods/saintinfo.c:375` | Heals all lost Endurance and Strength. Charisma +10 to 19. |
| `mods/saintinfo.c:378` | Heals 10% of lost Endurance and 10% of lost Strength. Agility +12 to 19, Bows +20 to 39, Riding +10 to 19. |
| `mods/saintinfo.c:380` | Heals 10% of lost Endurance and Strength. Agility +12 to 19, Bows +20 to 39, the other weapon skills +6 to 11, Riding +10 to 19. |
| `mods/saintinfo.c:381` | Heals 20% of lost Endurance and 10% of lost Strength. Charisma +6 to 11, Speak Common +30 to 59, Latin +20 to 39. |
| `mods/saintinfo.c:382` | No reputation in its help (Poland and Cracow: no such places in the game). |
| `mods/saintinfo.c:383` | May raise the reputation by 10 to 19 in Posen or Bromberg (nowhere else). Heals 20% of lost Endurance and 10% of lost Strength. Charisma +6 to 11, Speak Common +30 to 59, Latin +20 to 39. |
| `mods/saintinfo.c:383` | No reputation in Cracow, which is not in the game. |
| `mods/saintinfo.c:384` | In Pressburg, may raise the reputation by 15 to 24. Charisma +6 to 11, Intelligence +8 to 15, the skills of the weapons in hand +15 to 24, Speak Common +6 to 11, Riding +10 to 19. |
| `mods/saintinfo.c:387` | Heals 20% of lost Endurance and 10% of lost Strength. Charisma +10 to 19, Speak Common +25 to 49. |
| `mods/saintinfo.c:390` | Heals 40% of lost Endurance and 20% of lost Strength. Charisma +6 to 11. |
| `mods/saintinfo.c:393` | Heals 40% of lost Endurance and 20% of lost Strength. Healing +30 to 59. |
| `mods/saintinfo.c:396` | Charisma +6 to 11, Endurance +4 to 8, the skills of the weapons in hand +15 to 24, Flails +15 to 29. |
| `mods/saintinfo.c:397` | Flails instead of Riding. |
| `mods/saintinfo.c:398` | Charisma +6 to 11, the skills of the weapons in hand +15 to 24, Riding +25 to 49. |
| `mods/saintinfo.c:399` | Strength and Endurance +4 to 8, Speak Common and Woodwise +4 to 7, Healing +10 to 19, Artifice +12 to 23. |
| `mods/saintinfo.c:402` | Intelligence +12 to 23, Speak Common and Latin +12 to 26, Read & Write +15 to 29, Healing +10 to 19. |
| `mods/saintinfo.c:403` | No Religion and no reputation at monasteries and universities; instead Speak Common and Healing. |
| `mods/saintinfo.c:404` | In a town with a monastery or a university, may raise its reputation by 10 to 19. Intelligence +12 to 23, Latin +12 to 26, Read & Write +15 to 29, Religion +25 to 49. |
| `mods/saintinfo.c:405` | Heals 10% of lost Endurance and 10% of lost Strength. Charisma +10 to 19, Stealth, Healing and Speak Common +6 to 11. |
| `mods/saintinfo.c:408` | Heals all lost Endurance and 20% of lost Strength. Charisma +8 to 15, the skills of the weapons in hand +15 to 24, Speak Common +10 to 19. |
| `mods/saintinfo.c:411` | Charisma +10 to 19, Agility +12 to 23. At the party's next step in the overland view it is carried to the nearest town. |
| `mods/saintinfo.c:412` | No Endurance lost. |
| `mods/saintinfo.c:413` | Charisma +10 to 19, Agility +12 to 23. One time in two, the one prayed for loses 3 to 8 Endurance (not below 1). At the party's next step in the overland view it is carried to the nearest town. |
| `mods/saintinfo.c:414` | May raise the reputation by 15 to 24 in St. Joachimsthal, Bürglitz, Prag, Kuttenberg, Olmütz or Brünn (nowhere else). Perception and Strength +6 to 11, Intelligence +8 to 13, the skills of the weapons in hand +8 to 15, Riding +8 to 15. |
| `mods/saintinfo.c:415` | No reputation with nobles in its help. |
| `mods/saintinfo.c:417` | Heals 20% of lost Endurance. Charisma +6 to 11, Artifice +6 to 11, Speak Common +20 to 39. For a week no ambush takes the party by surprise (bandits and the like). |
| `mods/saintinfo.c:418` | No faster travel; instead, a week safe from ambushes. |
| `mods/saintinfo.c:419` | The party travels twice as fast for a week. Heals 20% of lost Endurance. Charisma +6 to 11, Artifice +6 to 11, Speak Common +20 to 39. |
| `mods/saintinfo.c:420` | Agility +8 to 15, Woodwise +8 to 15, Riding +10 to 19. For a week no ambush takes the party by surprise (bandits and the like). |
| `mods/saintinfo.c:423` | Charisma +10 to 19, Woodwise +6 to 11, Speak Common +25 to 49. |
| `mods/saintinfo.c:426` | Charisma +10 to 19, Woodwise +8 to 15, Speak Common +20 to 39. |
| `mods/saintinfo.c:427` | No reputation in its help. |
| `mods/saintinfo.c:428` | May raise the reputation by 10 to 19 in Groningen, Leer, Osnabrück, Zwolle, Deventer, Münster, Nymwegen or Elburg (nowhere else). Charisma +10 to 19, Woodwise +8 to 15, Speak Common +20 to 39. |
| `mods/saintinfo.c:429` | Charisma +8 to 15, Intelligence +6 to 11, Woodwise and Riding +6 to 11, Speak Common +20 to 39. |
| `mods/saintinfo.c:430` | No reputation in its help. |
| `mods/saintinfo.c:432` | Heals 20% of lost Endurance and 5% of lost Strength. Perception +6 to 11, Streetwise +6 to 11. |
| `mods/saintinfo.c:444` | Meant as protection against fire, but the game adds it to the armor instead, for good (with the fix "No invulnerability from fire protection" on, it does nothing). |
| `mods/saintinfo.c:445` | Meant as protection against fire, it gives the one prayed for armor +32 in battle instead, for good. |
| `mods/saintinfo.c:447` | Every member's worn body and leg armor +10 quality, for good, piling up with each prayer. |
| `mods/saintinfo.c:451` | May raise the reputation by 10 to 19 in St. Joachimsthal, Bürglitz, Kuttenberg, Olmütz or Brünn (nowhere else); in Prag it goes to a place picked at random. |
| `mods/saintinfo.c:455` | Every member: fire protection 90 for good (and an armor bonus, which the fix "No invulnerability from fire protection" cancels). |
| `mods/saintinfo.c:456` | Every member: fire protection 90 for good, and armor +48 for good (meant as protection against fire). |
| `mods/saintinfo.c:487` | This is the help the texts and the manual give St. Paul the Simple: the two Pauls are mixed up (the fix "The two St. Pauls do what their texts say" sets this right). |
| `mods/saintinfo.c:488` | This is St. Thomas Aquinas's help: the two Pauls are mixed up (the fix "The two St. Pauls do what their texts say" sets this right). |
| `mods/saintinfo.c:500` | A saint's help lasts about a day, unless said otherwise. |
| `mods/sainttexts.c:79` | Endurance (healing only) |
| `mods/sainttexts.c:84` | Endurance (healing only) |
| `mods/sainttexts.c:89` | Strength and Endurance (healing only) |
| `mods/sainttexts.c:105` | may double party travel speed |
| `mods/sainttexts.c:274` | prevents death from wounds for a period (strength never goes below 1) |
| `mods/sainttexts.c:290` | heals a small amount of Endurance |
| `mods/sainttexts.c:298` | a modest increase to Charisma, a good increase to all weapons skills |
| `mods/sainttexts.c:305` | Enhances ALL skills by a small amount plus one for every five points of virtue |
| `mods/sainttexts.c:310` | every skill by a modest amount |
| `mods/saves.c:64` | Ironman 3f2a |
| `mods/saves.c:67` | An Ironman game cannot go back to a save. To load another game, quit to the start screen. |
| `mods/saves.c:182` | Could not save. |
| `mods/saves.c:185` | Saved (an Ironman game keeps one save, saved over as you go) |
| `mods/saves.c:228` | The game does not allow saving while staying at an inn or camping: leave first (the days already spent are not given back). |
| `mods/saves.c:229` | The game does not allow saving during a battle. |
| `mods/saves.c:230` | The game does not allow saving on this screen: leave it first (saving works on a card, in the overland view or in a town). |
| `mods/saves.c:248` | The game does not allow loading here. |
| `mods/saves.c:257` | Difficulty is now set to ... |
| `mods/saves.c:270` | Loading is irreversable. Proceed? |
| `mods/saves.c:313` | That save could not be read. |
| `mods/saves.c:328` | Loaded %s: a battle, paused (Space resumes it) |
| `mods/saves.c:347` | Saved (an Ironman game keeps one save, saved over as you go) |
| `mods/saves.c:363` | Saved: %s (%s), in place of the old save |
| `mods/saves.c:364` | Saved: %s (%s) |
| `mods/saves.c:387` | use your horses |
| `mods/saves.c:403` | Autosave: battle |
| `mods/saves.c:435` | Ironman %04x |
| `mods/saves.c:437` | An Ironman game: it saves itself as you go, over one save. There is no going back. |
| `mods/saves.c:469` | Ironman: the game closes once it can save, after this battle. Close again to quit at once (the game then goes back to the last save). |
| `mods/saves.c:470` | Ironman: saving, then closing. Close again to quit at once (the game then goes back to the last save). |
| `mods/saves.c:491` | Start or load a game first. |
| `mods/saves.c:498` | Loading from here works once a game is running; until then, load from the start screen. |
| `mods/saves.c:500` | Buffer dealloc error |
| `mods/saves.c:502` | That save could not be read. |
| `mods/saves.c:522` | No quicksave yet (Ctrl+S makes one). |
| `mods/savesview.c:51` | %.14s %d %.3s |
| `mods/sfx.c:50` | an attack that misses |
| `mods/sfx.c:50` | a parried or blocked blow |
| `mods/sfx.c:50` | a blow that lands |
| `mods/sfx.c:51` | a heavy blow that knocks back |
| `mods/sfx.c:51` | a bow or crossbow shot |
| `mods/sfx.c:51` | a handgun shot |
| `mods/sfx.c:51` | a potion bursting |
| `mods/sfx.c:52` | a magical effect |
| `mods/sfx.c:52` | hidden enemies appear |
| `mods/sfx.c:52` | "cannot go there" |
| `mods/sfx.c:586` | %d sounds written to %s |
| `mods/sharedstore.c:121` | This inn's own store room holds too many kinds of items to join the store room shared by every inn (Darklands+): its items stay at this inn until some are taken out. |
| `mods/sheetview.c:212` | No %s is left in the party's pack. |
| `mods/sheetview.c:214` | %s cannot carry another kind of item. |
| `mods/sheetview.c:219` | The %s could not be handed to %s. |
| `mods/sheetview.c:233` | The %s is marked Keep: it will not be sold or dropped until you unmark it. |
| `mods/sheetview.c:233` | The %s may be sold and dropped again. |
| `mods/sheetview.c:242` | %s takes up the %s. |
| `mods/sheetview.c:243` | The %s cannot be readied. |
| `mods/sheetview.c:253` | %s drinks the %s. |
| `mods/sheetview.c:256` | The %s is marked Keep: unmark it before dropping it. |
| `mods/sheetview.c:272` | Left behind: the %s (%d). Undo takes them back. |
| `mods/sheetview.c:272` | Left behind: a %s. Undo takes it back. |
| `mods/sheetview.c:319` | Taken back: the %s (%d). |
| `mods/sheetview.c:319` | Taken back: the %s. |
| `mods/sheetview.c:320` | The %s cannot be taken back (no room). |
| `mods/sheetview.c:325` | %s takes up the %s. |
| `mods/sheetview.c:326` | The %s cannot be readied. |
| `mods/sheetview.c:330` | %s puts the %s away. |
| `mods/sheetview.c:336` | %s cannot carry another kind of item. |
| `mods/sheetview.c:338` | %s gives the %s to %s. |
| `mods/sheetview.c:338` | %s gives a %s to %s. |
| `mods/sheetview.c:339` | %s cannot take the %s. |
| `mods/sheetview.c:343` | The %s is marked Keep: unmark it before dropping it. |
| `mods/sheetview.c:350` | Left behind: the %s (%d). Undo takes them back. |
| `mods/sheetview.c:350` | Left behind: a %s. Undo takes it back. |
| `mods/sheetview.c:357` | %s drinks the %s. |
| `mods/teachers.c:27` | Blacksmiths' guild |
| `mods/teachers.c:27` | In the daytime, go to the blacksmiths' guild and choose "request a meeting with the guild's masters". |
| `mods/teachers.c:28` | Swordsmiths' guild |
| `mods/teachers.c:28` | In the daytime, go to the swordsmiths' guild and choose "request a meeting with the guild's masters". |
| `mods/teachers.c:29` | Armorers' guild |
| `mods/teachers.c:29` | In the daytime, go to the armorers' guild and choose "request a meeting with the guild's masters". |
| `mods/teachers.c:30` | Bowyers' guild |
| `mods/teachers.c:30` | In the daytime, go to the bowyers' and gunsmiths' guild and choose "request a meeting with the guild's masters". |
| `mods/teachers.c:31` | Artificers' guild |
| `mods/teachers.c:31` | In the daytime, go to the artificers' guild and choose "request a meeting with the guild's masters". |
| `mods/teachers.c:33` | The guild only lets in a party with enough fame, reputation in this town and Charisma  |
| `mods/teachers.c:33` | (otherwise its street leads only to the shop). While the guild is split by a dispute, its masters want help with that first. |
| `mods/teachers.c:111` | While the guild is split by a dispute, its masters want help with that first. |
| `mods/teachers.c:121` | A guild here is split by a dispute, and its masters want help with that first. |
| `mods/teachers.c:124` | The guild would receive the party now, in the daytime. |
| `mods/teachers.c:124` | The guild would not receive the party yet (it needs more fame, reputation here or Charisma). |
| `mods/teachers.c:132` | In the daytime, go to the university and choose "a teacher you can hire". |
| `mods/teachers.c:133` | They may say no (it depends on Perception, Intelligence, Latin, the party's best Alchemy and fame). A refusal shuts their door for a week and lowers the party's reputation here a little. |
| `mods/teachers.c:140` | Go to the monastery and choose "inquire about tutoring in various disciplines". |
| `mods/teachers.c:141` | Only for a party of good name (Virtue, Charisma, reputation here), and only while no other teacher is hired in the town. After a refusal, wait two days. |
| `mods/teachers.c:148` | In the daytime, go to the physician and choose "beg him to allow you to be his students". |
| `mods/teachers.c:149` | He takes no one who already heals better than he does himself (his own Healing changes from week to week; visiting him shows it). Some years he takes no students. |
| `mods/teachers.c:160` | His own Healing is %d this week, and the party's best is %d: he will not teach you. |
| `mods/teachers.c:161` | His own Healing is at most %d, and the party's best is %d: he will not teach you. |
| `mods/teachers.c:162` | He takes no students this year. |
| `mods/teachers.c:164` | He takes no students at the moment. |
| `mods/teachers.c:165` | His own Healing is %d this week, and the party's best is %d: he will not teach you. |
| `mods/teachers.c:166` | His own Healing is %d this week: he will teach you. |
| `mods/teachers.c:167` | His own Healing is at most %d, and the party's best is %d: he will not teach you. |
| `mods/teachers.c:168` | His own Healing is %d to %d this week (visiting him shows which): he teaches only if it is at least the party's best, %d. |
| `mods/teachers.c:173` | In the daytime, go to Astrologists' Lane and choose "receiving additional instruction in alchemy". |
| `mods/teachers.c:174` | They may say no (it depends on Charisma, Speak Common, the party's best Alchemy and reputation here). After a refusal, wait two and a half days. |
| `mods/teachers.c:185` | Another teacher is hired in the town, so the monks will not teach for now. |
| `mods/teachers.c:189` | Already hired: the teacher is at the inn now. |
| `mods/teachers.c:226` | The guild's masters |
| `mods/teachers.c:227` | Here: choose "inquire about special training". |
| `mods/teachers.c:228` | They may say no (it depends on Charisma, reputation here and fame); they always agree if the guild owes the party a favor. |
| `mods/teachers.c:231` | A bug in the original game: this guild's teacher teaches no skill at all. |
| `mods/travel.c:243` | No way there from here. |
| `mods/travel.c:269` | how far is it |
| `mods/travel.c:366` | Travel stopped: the party could not get any further this way. |
| `mods/travel.c:516` | the marked place |
| `mods/war.c:98` | pics\gate4.pic: No such file |
| `mods/war.c:147` | ...hidden option |

