# Darklands+ changelog

## 1.4 (not yet released)

What changed since 1.3.1.

- **Level map (Tab), floors:** in a castle, mine or dungeon with more than one floor, a small chart at the top right, a square a floor: the floors the party has been on, the stairs seen between them, the party's floor outlined, and "?" for a floor its stairs lead to that nobody has been on; a click on a floor these stairs lead to walks the party there. The map also keeps what the party saw through a card in the middle of the dungeon (a riddle, a talk).
- **Camp:** Guard the camp has a picture of its own (the watch's lantern), and under it the chance that the lord's men find the party, before the next day and the day after; Train or study, never possible in the wild, is gone.
- **Praying:** the favor to spend stops where the chance reaches its best (99% at most); the rest of the bar is hatched, kept.
- **Saints on the character sheet** can be picked by what they raise: a row of the attributes' and skills' shields and the town's opinion (all saints that raise Perception, say), the saint who gives the most of it first. Each saint's help shows under its name as shields, the most first, with one to three chevrons on each for how much it gives against what the other saints give (red, reversed, for what it costs); the Virtue it asks is a shield and number beside its name, greyed when nobody in the party has as much (the same on the cards' list of saints and the prayer screen). The saint clicked shows each member's best chance of being heard with the least favor it takes, and Pray, and what its help does an effect a line: shields that share a value side by side, the value once.
- **A relic found deep in a mine** (dwarfs, kobolds, vulcans) is as good as the relic itself: the original made every one quality 35 and not its own kind of thing, so the saints' weapons among them could not be wielded. (61 fixes now.)
- **World map, a town's panel:** its saints (those the party knows greyed), formulae as potions (known ones greyed) and teachers as the skills' shields, as in the margin; its boats as a dock, a boat and an arrow (leaving, arriving, both ways) before the towns.
- **Alchemy:** a chart of the chance for every number of batches the ingredients allow (a click sets it), and under it what may come of the brew in pictures: the potions, the ingredients lost, a stink or an explosion at the inn. The chance's parts are shown as shields and signs. Formulae can be starred (first in the list) or put aside (last). The list says how many potions the ingredients make, and the Philosopher's Stone's plaque what its quality adds.
- **Shields for attributes and skills, in place of their names:** the years' toll and "Lost for good" show each member's age (an hourglass) and each attribute or skill lowered as its shield with what it was and is; the town's teachers show who would learn which skill, by its shield, and from what; the margin's blessings and potions show each raised attribute or skill as its shield and how much; a card's chance names what it rests on beside each shield. In texts too, wherever an attribute or skill is named, changed or asked for: the inn's and camp's days, a weapon's skill and the Strength it needs, the load, the prayer screen, a profession's needs at character creation, the teachers' and audiences' conditions, the potions, the battle log and the skill raised in a battle; the pointer on a shield names it.
- **The pointer:** the game's armored gauntlet drawn anew, crisp at any size: a fist while something is dragged; on its cuff the pause while a battle is paused, crossed blades over a foe a click would attack (a member chosen), an hourglass while Go to walks (F12 > Settings: the game's own little picture if preferred).
- **Battle, a foe's tooltip and Enemy Info:** each of your weapons shown as its picture, then whether it pierces the foe's armor, does half damage or is mostly deflected.
- **A shop's Formulae tab:** each formula's ingredients as pictures, what the party carries against what a batch needs and the price here; a click on one finds it in For sale. The +1 and +5 buttons are gone.
- **World map:** the quests alone at the left; at the top right, finding a town (by name, the towns with something, where to buy an ingredient or any ware) and, merged, what the map shows (the layers and the kinds of place); each panel folds away. The party's pulsing mark centers the map on it. The ingredient and ware finders sort by a column's head (name, carried or quality, nearest), and the ware finder lists every ware seen until a name narrows it.
- **Report a problem** (F12 > About): one click makes a zip of what a bug report needs (this session's log and the last one's, the settings, the two newest saves; your own name taken out of the folder paths in them), opens its folder and the GitHub bug form with the version, system and preset filled in; drag the zip into the form (posting takes a free GitHub account).
- **Aging:** pointing at a member's age on the character sheet tells what the years will take next, and by five years on.
- **The Philosopher's Stone on sale:** each town's alchemist and university always sell the same quality; the town's row in the margin shows the best (dim when no better than the party's), and the card's option to buy one is dimmed where it would gain nothing (a failed attempt still costs time).

## 1.3.1

Fixes to 1.3, nothing else. The first start makes the game's code again (a minute or two), as after installing.

- **A door or passage a card opens now opens** (GitHub issue 2): in the mines, after the dwarves' riddle and the talk, the party stood again before the same shut iron door and could not reach the next floor. The translated program compared the game's routines without the place the program sits in memory, so the card's "the door slides open" never reached the floor; the translation is fixed.
- **Saving inside a castle, mine or dungeon** kept the level's traps, secret doors, alarms and such doors as they are: through the same fault, a battle save turned every one into a trapdoor the whole party had to stand on, and after loading it the dwarves' door showed no riddle. Battle saves made by earlier versions are put right as they load.
- **Enemies spring the dart and spell traps they walk into**, as in the original game (the same fault kept them from it).
- **Fighters find their way around obstacles:** party members no longer get stuck for good. One who had waited a while behind a friend or a corner was told from then on that there was no way from where it stood: the three behind the leader stayed put, and in a group that one member made every order of the group fail. It lasted until the next fight, or until the switch was turned off.
- **Scrolling a battle or dungeon by resting the pointer on an edge** (or with Shift) no longer flickers: each step showed the field without its people for a moment, and at the edge of the field, where the view no longer moves, at every step.

## 1.3

What changed since 1.2. Each can be switched off in F12 > Settings, or is a choice there.

**Battles**

- **Improved combat** (on by default): the party fights on sensibly between orders. Shooters open fire by themselves, take a new mark when theirs falls, step aside for a clear line, and draw their melee weapon when a foe closes in or a friend a few paces off is fighting one. Members with no orders join a fight beside them. One switch replaces 1.2's separate shooting switches.
- **Orders in one click:** select a member (on the field or their box), then click an enemy or a spot. The orders are buttons with pictures at the foot of the field, a bow or sword on each shooter's box chooses shooting or melee, a label by the pointer says what a click will do, and a shot's line shows what blocks it.
- **Walking in a crowd:** fighters no longer stand still for seconds behind someone, enemies no longer take long detours, and planning their routes no longer makes the battle stutter.
- **Battle info:** the to-hit numbers are the ones the game uses (1.2 had attack and defense swapped), the weapons named are the ones in hand (1.2 named others), and each of your weapons against an enemy's armor is colored: pierces, half damage, or mostly deflected.

**Cards, towns and the party**

- **Cards say who tries an option and what it rests on:** the member's head, the skills and attributes as heraldic shields, the town's opinion and the party's fame. Where any member may speak for the party, a click on another member's chance has them speak (an alchemist's formula trade too). An option on the way to one of the party's quests in town (a reward waiting, papers to take from an office) carries the quest's picture, and the town gate warns when its guards may stop the party.
- **Screens redrawn in period style:** alchemy as an alchemist's workbench with the Philosopher's Stone pictured; the inn and camp as a ledger that names the potions made and shows the hours going by (1.2's page looked frozen); the shop at its counter, the store room in a cellar, Party Info, Save and Load, marching order and enemy info.
- **Shops and the character sheet:** items in sections; the load a weapon or armor would bring before you buy it; ingredients ten at a time, with how many the party has; a spare horse given to a member with Ride, a drag, or their number. Items can be marked Keep or Sell, and the shop's Sell spare gear sells those marked Sell.
- **Ammunition that runs out** (with the shared pack): each member's arrows, quarrels, balls and thrown weapons are their own.
- **"Lost for good"** no longer shows after a battle for a member whose skill a saint has raised for a while (1.2 showed it after every battle, the boosted skills "lowered").
- **Go to** works by night, says why a place is shut, and shows the party walking instead of flashing the cards on the way.
- **Calling on a saint:** the favor to spend on a bar with the best chance marked, the reason when there is no chance, the Virtue each saint asks, and prayers from the character sheet.
- **Docks:** a small map of where the boats go, with each fare. **More day jobs:** nine jobs the original never offers turn up in towns with the right places.
- **Right margin:** the party's fame, the town's reputation with what changed it, whether its watch is on alert, and the formulae and teachers to be had in town as pictures.

**The world map**

- **Quests and towns live on the world map** (the Journal's pages for them are gone): boat routes along the rivers and coasts, windows to find alchemical ingredients or any ware, towns on wax seals, several quests at one place circling it, the way the party would take, Snap to party, and a view that stays on the map.

**Look and sound**

- **Period ornaments:** plaited frames and gilt corners, clocks to choose for passing time, skills and attributes as heraldic shields everywhere, ingredients as the alchemists' signs, potion grades as chevrons, and Comic Sans as an eighth typeface (we are so sorry).
- **Tooltips** open beside what they describe, never over it or the pointer.
- **Sound:** the whole soundtrack to play in Settings; previews of each device's music and battle sounds; evener music timing; the PC speaker heard at last (1.2 played nothing on it). The MT-32 and the CM-32L are one choice, and MIDI out is a device of its own.

**Settings and saving**

- **One Settings page** with a picture beside each section. Presets are Full Original, Purist and Darklands+; easier play comes as built-in mods (F12 > Mods, with a load order to drag).
- **Saving and loading** in the game's own Save and Load screens, redrawn; F12 no longer has a Saves page. The first start of each version backs your saves up.
- **Ironman** (for a new game): one save that the game keeps itself, with no loading back.
- **Controller button prompts**, drawn as an Xbox, PlayStation or Nintendo controller has them.

**Restored content** (F12 > Reconstructed Content, experimental, off by default)

- City politics, cities at war, the "Fehde!" ambush and a town's hospice: written by MicroProse and never switched on.

**Bugs of the original game**

- 23 more fixed (60 in all), most from the list Arkana Mechanika (Darklands Restoration) keep: climbing a city wall at night, mounts that never helped outrun wolves, a church's tenth that blessed nobody, an audience "fixed for tomorrow" that never came, bands of enemies missing from battles, shots stopped by clouds not in the way, loot lost past 255 of a kind, ordinary hits that made no sound, and more.
- The cave south-east of Basel, which lies off the map, is moved to where the party can walk onto it (1.2 had moved it onto the map's last row, which the party cannot walk onto).

**Withdrawn**

- The New Player Guide, until every statement in it has been checked in play.

## Since 1.0, most important first

Every change can be switched off. **F12 > Settings > Helpers** offers presets: **Buggy Purist** (the game as it was), **Purist** (its bugs fixed), **Purist + interface**, **Semi-Purist**, **Darklands+** and **Easy**, then every switch in one table.

- **The best member speaks for the party.** Every card test the game makes of "the leader" (talking your way past guards, a lore, a perception check...) is now made by whoever in the party does it best. Their name and chance are shown beside the option, and shops price by the party's best charisma. No more swapping the leader before every conversation.
- **Most of the game's screens are rebuilt for the mouse and modern screens:** cards, shops, the character sheet, loot, the inn's store room, the inn room and camping, alchemy, calling on a saint, Party Info, save and load, marching order, character creation and party selection, Enemy Info and the game's menus. The game still decides everything; each screen can go back to the original. The character sheet is the one place for the party's things, skills, saints and formulae (the F12 window's Inventory, Party, Alchemy and Saints pages are gone, their best parts moved onto it: a gold dot for the party's best at each skill and everyone's value under the pointer, a value raised by a saint in blue with how long it lasts, every formula with its ingredients, batches and chances, all 23 ingredients under the list, a weapon's or armour's full numbers when the pointer rests on it), and the world map the one place to choose a journey (its Find a place lists the nearest first, with the time on the way, and takes a name with or without its accents: Koeln finds Köln).
- **Teachers you can find.** A lectern marks every card option that brings a teacher to the inn: the guilds' masters, the university, the monks, the physician, the alchemists. Pointing at it says what they teach, how well (the chance a day's lesson adds a point, up to the teacher's own ability), the real fee, and what it takes (a guild's hall receives only a party of some standing; the physician takes no students some years). Each town's teachers are listed in the margin, on the map and in Go To.
- **Anything lost for good is said, in a window to click through:** an attribute's maximum or a skill lowered, armour worn down or ruined by acid or fire. **Characters age in play**, a little each month past thirty, spread from what character creation takes every five years.
- **Cards explain themselves.** Beside its chance of success, each option now names who would try it. A greyed option says why it can't be chosen and what would make it possible, with 280+ reasons read from the game's own card scripts. Options that start a fight are marked with crossed swords.
- **One pack for the whole party.** Each member keeps only what they use and their own potions; everything else goes into a shared pack, and only what is in use weighs. Darklands+ hands an item to the right member whenever the game needs one (lock picks, a mount...). **Keep** stops an item from ever being sold or dropped.
- **Quests you can follow.** Each task has a short title, its deadline and a picture of its kind, in the margin, the journal and the world map. Quest pictures mark the place on the travel map, and **Travel there** goes to it; in a town, Go To (G) marks the house a task leads to (a reward's giver, the office that keeps papers to be got). A town that bars you or whose watch wants you shows a sign beside its name.
- **Go To (G), redrawn.** Every place in a town has a picture (the Fugger's lilies and the Medici's red balls as they were about 1400, a Hanseatic cog in the League's red and white ...), grouped as Merchants, Quest givers, Religion, Learning and healing, and Around town; every place keeps its tile in every town (greyed where the town lacks it), so each is always in the same spot. Each place is a tile with a line for each thing worth knowing now: a task giver's chance of a task (or the date until it has none, or a reward waiting), your own task there, the teacher to hire and how far they teach, a shop's wares, what you learnt of the physician or the alchemists. The rest is under the pointer, in plain words.
- **Saints that do what their texts say.** 39 saints whose help, as coded, strayed from their own description now follow it: St. Christopher, St. Drogo and St. Wilfrid make the party travel faster for a week (fewer days on the road), St. Eustace through woods; St. Charity, St. Erasmus and St. Pantaleon protect the party against fire; St. George, St. Michael, St. Longinius, St. Maurice and St. Eligius better weapons and armour, St. Dunstan metal armour, St. Crispin the rest; St. Nicholas and St. Theodore give Riding, not Flails; St. Lutgardis's ecstasies cost Endurance; reputation goes to the saint's own lands ... (a switch, "sainttexts"). Beside each saint's description (the character sheet, the prayer screen) is what its help does, with numbers ("Endurance +8 to 15, every weapon skill +15 to 29"; a help lasts about a day unless said otherwise, noted beside each help that wears off), what the text still promises that the game cannot (reputation in Spain or Sweden ...), and whether encounters call for the saint. The whole table, and the reasons, are in docs/SAINTS.md.
- **The chronicle by day.** F12 > Journal > Chronicle keeps every text the cards showed; on its left is now a list of the days, by month, each opening to the places of that day. A day shows its texts in the order read, under the name of each place, with the hour, and Day before / Day after leaf through. The search narrows the list to the days where the words are found.
- **Where to learn each saint.** Every library teaches four saints, fixed for the whole game: the character sheet names the libraries the party has been to that teach a saint it lacks, and the margin and the world map list what a library teaches once the party has been there. Likewise each alchemist sells four formulae for the whole game, and the sheet names those that sell a formula the party lacks. (The Easy preset's "What unvisited places offer" tells it for the places not yet visited too.)
- **A new start screen** in the game's own art: **Continue** loads the newest save in one click (it names where, when and who), Load lists the saves to choose from, then a new party, Quickstart, Settings and Quit.
- **Character creation shows the whole career tree.** Every profession, by kind, with the ones open now in dark type. Pointing at one says what it needs (the game's own test, in words), what this character still lacks, what it leads to, and what it teaches, with how much room the skill points have afterwards. A childhood shows which professions it opens at first.
- **Arrows, quarrels, balls and thrown weapons never run out.** These can be switched off separately. Thrown weapons also go back to their throwers after a battle.
- **Smarter shopping and alchemy.** Shops compare a weapon or armour with what the member has now. **Sell spare gear** sells every unused weapon, armour and shield at once. The character sheet's Formulae show all 23 ingredients, those the party lacks greyed and what a formula calls for ringed; alchemy shows where a missing ingredient is sold, and a shop selling ingredients buys what one or five more batches of a formula need. The margin shows what a town offers (university, cathedral, alchemist, physician, docks, bank) and its notable shops, and the world map finds every town with one ("Towns with a university", "with an armourer of 30q+" ...).
- **One store room for every inn:** whatever you leave with one innkeeper is waiting at every other.
- **Potions with pictures:** each shows the potion's colour, an emblem of what it does, and one to three dots for its grade. Each says what it really does, from the game's code: the weapon oils work on every weapon, bows and thrown ones too, for about a day; each oil works with the weapons the manual names, and Great Power and Hardarmor do what the manual says.
- **Battles that read better:** better pathfinding for the party and the enemies; a member whose foe falls turns on one close by instead of walking round the whole melee to help the most wounded friend; a thin line while a shooter reloads and aims; a skill raised rises in gold over the fighter's head; bars only over the hurt (Alt shows all); a click sends a member better with a bow than with a blade to shoot; a click just beside a foe (its hot spot is only 8 pixels wide) is taken as on the foe, and a spot clicked on the ground before it no longer sends the member "walking" beside the foe; the right (or middle) button held and moved drags the view, a right click still opens the menus; a step of the view slides instead of jumping; 1 to 5 choose a member without moving the view (twice: the view goes to them); the field stands still a second, not three and a half, when the last foe falls.
- **F12 > Guide:** a short explanation of how the game works, for new players.
- **Fixed, a bug of Darklands+ 1.0 to 1.1.1:** with the Blessings fix on, a saint's lowering of an attribute (several saints lower Perception, Intelligence, Agility or Charisma for a day as the price of their help) never wore off. It now does; points already lost this way can be put back in F12 > Editor (shown once "Editing tools" is ticked in F12 > Settings > Helpers).
- **Bugs of 483.07 fixed:**
  - card options wrongly greyed or offered (a bribe you couldn't pay, a rope climb refused with a rope in the pack, a sally port whose lines ran the wrong options), and Clothmakers' Street, whose shop (clothing and soft armour) was switched off by day and sold only at night, to a sleeper woken for a point of reputation;
  - inn teachers that charged "-4096pfs" and taught nothing;
  - debt notes that were never where the merchant said (always in another office above the marketplace);
  - a formula's risk the wrong way round: a failed low-risk batch at an inn blew up three times in four, a high-risk one half the time;
  - worn armour that dragged identical spares down with it;
  - the bishop's errands given out as the merchants' (the merchants paid, the canon gave a new one at every visit, his relic could be taken again and again);
  - errands sent to a pagan altar the party had destroyed, which could never be done;
  - a tomb whose thing went to the party that lost the fight for it, not the one that won;
  - in a town without a council, the lord's rewards for his tasks could never be collected (only a council's hall pays them); the lord now pays at an audience;
  - weapon oils that never wore off and overflowed; Hardarmor, Firewall, St. Dunstan's and St. Patrick's armour that never wore off and piled up (all now last about a day, as the manual says);
  - Great Power's quality points added to damage and penetration instead (so the second grade gave +1 damage and only +6 penetration); it now raises the weapon's quality;
  - weapon oils that worked with any weapon (Deadly Blade on maces, Strongedge on bows), and Hardarmor that thickened armour by up to three levels and never raised its quality;
  - inn teachers who took a skill past their own ability, to 99;
  - the party's standing with the Fugger, the Medici and the Hanse, which a saved game did not keep (a load kept the last game's, or none after a restart);
  - a saint called in a battle whose help never wore off (and whose price was never given back);
  - healing that took a boost away: Essence o'Grace drunk while Ironarm ran brought Strength back down towards its maximum;
  - St. Paul the Apostle and St. Paul the Simple each gave the other's help (the Apostle's own was in the game but never called);
  - St. John Nepomuk called on in Prag raised the reputation of a place picked from whatever lay in memory, and St. Cecilia called on away from any town wrote it into memory that is not a place's;
  - St. Crispin's +10 armour quality, and the protection against fire of St. Polycarp, St. Charity, St. Erasmus and St. Pantaleon, stayed for good (the last three as +32 armour, or nothing at all): now a day each, the fire saints as fire protection;
  - a bowyer woken at night sold the smiths' swords, axes and armour and no bows (found by Arkana Mechanika, Darklands Restoration);
  - a confession in a town where the party's reputation is past 109 took no time, or turned the clock back (found by Arkana Mechanika).
- **Also:**
  - the opening plays whole; a click goes on to the lady's welcome (the gargoyles' music stops for it), a second to the title;
  - the world map washes out the towns not yet visited (their saints and formulae are known once visited) and the quests glow;
  - the margin tells the hour by the church bells (Prime, Vespers ...) as the cards do; Go To reaches the town hall, the prison and the barracks, remembers a town's physician and alchemists, and shows who has work: beside each giver of tasks, the chance that asking brings one, or until when none (a refusal shuts the Fugger's door for three days, an audience tried the council's for a week);
  - money shown as gold, silver and copper coins;
  - a new save can take the place of an old one (a click on it in the Save page), and the Load page deletes a save;
  - the level map (Tab) walks the party to a spot you click;
  - night-time thieves and watch in towns can be made rarer, and the margin says when the streets are dark (from 19:00, the game's own hour, not 21:00) with the party's chance of slipping past the cutthroats;
  - experimental, off by default: a shooter whose line is blocked steps aside to a spot with a clear one, or walks towards the foe until there is one, and can be sent at any foe;
  - archers can optionally shoot past their own fighters;
  - Z fast-forwards a battle (six times the pace) and Z again slows it back down;
  - the end of a battle says "Victory! The battle is won." instead of holding a still field for eight seconds;
  - in a castle, dungeon or mine, the battle margin can say how many foes still wait there, floor by floor (Foes left: a peek, so only in the Easy preset), and the battle pauses when a group of them comes out to fight;
  - a witches' Sabbat the party has heard of (its night, its place) goes into the tasks, the place marked on the map;
  - choosing the party: the party's best in each skill outside weapons, weak ones in red, and what each is for;
  - a party panel with bars and your own portraits; in a battle each member's order in words (attacks, flees, no line, stairs, picks ...), pointing at it says more;
  - smoother travel (no stop every second);
  - a crash on loading with the side margins on is fixed.

## Per version

- **1.3 (to come):** Improved combat and one-click orders, cards that show who tries an option and what it rests on, the restored city politics, cities at war, Ironman, the soundtrack and sound previews, the world map's finders, and more of the original's bugs fixed.
- **1.2 (previews):** the rebuilt screens, the best member, the shared pack and store room, quests at a glance, endless ammunition, potion pictures, the Guide, the card fixes and hints.
- **1.1.1:** fixed a crash on loading with the side margins on. Smoother battle-map scrolling, thrown weapons returned after battles, and missed shots in the battle log.
- **1.1:** cards drawn by Darklands+ with clickable options, chances and an illuminated initial.
- **1.0.1:** travel no longer stutters once a second.
- **1.0:** first release.

The full notes are in [docs/release_notes.md](docs/release_notes.md).
