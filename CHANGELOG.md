# Darklands+ changelog

## Since 1.0, most important first

Every change can be switched off. **F12 > Settings > Helpers** offers presets: **Buggy Purist** (the game as it was), **Purist** (its bugs fixed), **Purist + interface**, **Semi-Purist**, **Darklands+** and **Easy**, then every switch in one table.

- **The best member speaks for the party.** Every card test the game makes of "the leader" (talking your way past guards, a lore, a perception check...) is now made by whoever in the party does it best. Their name and chance are shown beside the option, and shops price by the party's best charisma. No more swapping the leader before every conversation.
- **Most of the game's screens are rebuilt for the mouse and modern screens:** cards, shops, the character sheet, loot, the inn's store room, the inn room and camping, alchemy, calling on a saint, Party Info, save and load, marching order, character creation and party selection, Enemy Info and the game's menus. The game still decides everything; each screen can go back to the original.
- **Teachers you can find.** A lectern marks every card option that brings a teacher to the inn: the guilds' masters, the university, the monks, the physician, the alchemists. Pointing at it says what they teach, how well (the chance a day's lesson adds a point; there is no limit but 99), the real fee, and what it takes (a guild's hall receives only a party of some standing; the physician takes no students some years). Each town's teachers are listed in the margin, on the map and in Go To.
- **Anything lost for good is said, in a window to click through:** an attribute's maximum or a skill lowered, armour worn down or ruined by acid or fire. **Characters age in play**, a little each month past thirty, spread from what character creation takes every five years.
- **Cards explain themselves.** Beside its chance of success, each option now names who would try it. A greyed option says why it can't be chosen and what would make it possible, with 280+ reasons read from the game's own card scripts. Options that start a fight are marked with crossed swords.
- **One pack for the whole party.** Each member keeps only what they use and their own potions; everything else goes into a shared pack, and only what is in use weighs. Darklands+ hands an item to the right member whenever the game needs one (lock picks, a mount...). **Keep** stops an item from ever being sold or dropped.
- **Quests you can follow.** Each task has a short title, its deadline and a picture of its kind, in the margin, the journal and the world map. Quest pictures mark the place on the travel map, and **Travel there** goes to it; in a town, Go To (G) marks the house a task leads to (a reward's giver, the office that keeps papers to be got). A town that bars you or whose watch wants you shows a sign beside its name.
- **Where to learn each saint.** Every library teaches four saints, fixed for the whole game: F12 > Saints names the nearest library teaching each saint the party lacks, and the margin and the world map list what a town's library teaches. Likewise each alchemist sells four formulae for the whole game: F12 > Alchemy names the nearest alchemist selling each formula the party lacks.
- **A new start screen** in the game's own art: **Continue** loads the newest save in one click (it names where, when and who), Load lists the saves to choose from, then a new party, Quickstart, Settings and Quit.
- **Character creation shows the whole career tree.** Every profession, by kind, with the ones open now in dark type. Pointing at one says what it needs (the game's own test, in words), what this character still lacks, what it leads to, and what it teaches, with how much room the skill points have afterwards. A childhood shows which professions it opens at first.
- **Arrows, quarrels, balls and thrown weapons never run out.** These can be switched off separately. Thrown weapons also go back to their throwers after a battle.
- **Smarter shopping and alchemy.** Shops compare a weapon or armour with what the member has now. **Sell spare gear** sells every unused weapon, armour and shield at once. Alchemy shows where a missing ingredient is sold, and a shop selling ingredients buys what one or five more batches of a formula need. The margin shows what a town offers (university, cathedral, alchemist, physician, docks, bank) and its notable shops, and the world map finds every town with one ("Towns with a university", "with an armourer of 30q+" ...).
- **One store room for every inn:** whatever you leave with one innkeeper is waiting at every other.
- **Potions with pictures:** each shows the potion's colour, an emblem of what it does, and one to three dots for its grade. Each says what it really does, from the game's code: the weapon oils work on every weapon, bows and thrown ones too, for about a day.
- **Battles that read better:** better pathfinding for the party and the enemies; a member whose foe falls turns on one close by instead of walking round the whole melee to help the most wounded friend; a thin line while a shooter reloads and aims; a skill raised rises in gold over the fighter's head; bars only over the hurt (Alt shows all); a click sends a member better with a bow than with a blade to shoot; the field stands still a second, not three and a half, when the last foe falls.
- **F12 > Guide:** a short explanation of how the game works, for new players.
- **Fixed, a bug of Darklands+ 1.0 to 1.1.1:** with the Blessings fix on, a saint's lowering of an attribute (several saints lower Perception, Intellect, Agility or Charisma for a day as the price of their help) never wore off. It now does; points already lost this way can be put back in F12 > Editor.
- **Bugs of 483.07 fixed:**
  - card options wrongly greyed or offered (a bribe you couldn't pay, a rope climb refused with a rope in the pack, a sally port whose lines ran the wrong options);
  - inn teachers that charged "-4096pfs" and taught nothing;
  - debt notes that were never where the merchant said (always in another office above the marketplace);
  - a formula's risk the wrong way round: a failed low-risk batch at an inn blew up three times in four, a high-risk one half the time;
  - worn armour that dragged identical spares down with it;
  - the bishop's errands given out as the merchants' (the merchants paid, the canon gave a new one at every visit, his relic could be taken again and again);
  - errands sent to a pagan altar the party had destroyed, which could never be done;
  - a tomb whose thing went to the party that lost the fight for it, not the one that won.
- **Also:**
  - the opening plays whole; a click goes on to the lady's welcome, a second to the title;
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
  - in a castle, dungeon or mine, the battle margin says how many foes still wait there, floor by floor, and the battle pauses when a group of them comes out to fight;
  - a witches' Sabbat the party has heard of (its night, its place) goes into the tasks, the place marked on the map;
  - choosing the party: the party's best in each skill outside weapons, weak ones in red, and what each is for;
  - a party panel with bars and your own portraits; in a battle each member's order in words (attacks, flees, no line, stairs, picks ...), pointing at it says more;
  - smoother travel (no stop every second);
  - a crash on loading with the side margins on is fixed.

## Per version

- **1.2 (previews):** the rebuilt screens, the best member, the shared pack and store room, quests at a glance, endless ammunition, potion pictures, the Guide, the card fixes and hints.
- **1.1.1:** fixed a crash on loading with the side margins on. Smoother battle-map scrolling, thrown weapons returned after battles, and missed shots in the battle log.
- **1.1:** cards drawn by Darklands+ with clickable options, chances and an illuminated initial.
- **1.0.1:** travel no longer stutters once a second.
- **1.0:** first release.

The full notes are in [docs/release_notes.md](docs/release_notes.md).
