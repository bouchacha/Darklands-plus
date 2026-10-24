// guide.cpp - F12 > New Player Guide: how Darklands works, in short, for a new player (the game was sold
// with a thick manual and does not explain itself). The game's own rules as Darklands+ reads them
// from the program (Endurance and Strength, the inn's numbers, alchemy's chance, armor against
// penetration ...), and the advice players have long given one another (the GOG and Steam forums,
// the hint book), no story. A line that starts with '+' depends on something Darklands+ adds:
// it is drawn in its own color, so a player of the original can tell it apart.
#include "imgui.h"
#include <string.h>
#include <initializer_list>

#include "../runtime/config.h"

static const ImVec4 PLUS_INK(0.10f, 0.30f, 0.58f, 1);     /* what Darklands+ adds (the legend says so) */

void ui_guide_tab(void)
{
    if (!config_get_int("guide.seen", 0)) { config_set_int("guide.seen", 1); config_save(); }     /* (the margin's hint is not needed any more) */
    ImGui::TextWrapped("Darklands has no tutorial: this page sums up how it works, with the advice players have long given one another. "
                       "F11 (a Darklands+ addition) lists the keys and clicks of the current screen at any time; a right-click opens the game's menus.");
    ImGui::TextColored(PLUS_INK, "Lines in this color depend on something Darklands+ adds (most can be switched off in F12 > Settings).");
    ImGui::Spacing();
    auto topic = [](const char *title, std::initializer_list<const char *> lines, bool open = false) {
        if (ImGui::CollapsingHeader(title, open ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
            ImGui::Indent();
            for (const char *l : lines) {
                bool plus = l[0] == '+';
                ImGui::Bullet();
                if (plus) ImGui::PushStyleColor(ImGuiCol_Text, PLUS_INK);
                ImGui::TextWrapped("%s", plus ? l + 1 : l);
                if (plus) ImGui::PopStyleColor();
            }
            ImGui::Unindent();
            ImGui::Spacing();
        }
    };
    topic("First steps", {
        "A new party has little money and light armor. Buy the best armor the purse allows before anything else (studded leather or mail if you can): a weapon whose penetration is below an armor's hardness is mostly turned aside (Battle, below).",
        "The usual way to toughen a new party: walk a town's side streets and docks at night. Thugs and cutthroats attack; each fight raises weapon skills quickly, and their weapons and armor sell well (cloth armor sells for little). The worst the night watch does is fine you. Go back to the inn to rest between rounds.",
        "+Darklands+ can make these night ambushes rarer (F12 > Mods: Night encounters in the streets); leave it as the original if you mean to train this way.",
        "Quests (paid work) come from a town's merchants, foreign traders and pharmacists (at the marketplace), the Fugger, the Medici, the Hanse, the town council and the cathedral: robber knights to kill, items to fetch. The banks' and the council's pay best. Note down what you were asked to do and where.",
        "+The left margin lists the party's quests, and the world map (M) marks where each one leads.",
        "Rest at an inn when Strength is low, and set out with it full. In the wilds, camping costs nothing.",
        "Keep the party's Virtue up and never attack a priest or a monk: the Church remembers. Cleared streets and good deeds raise the town's opinion of the party.",
        "A good party has a leader with high Charisma who speaks the common tongue well, someone who reads Latin, and a woodsman (Woodwise) for the road. Nürnberg, in the middle of the land, is a good home town.",
        "+Sell what nobody uses: in a shop, the Sell spare gear button sells at once every weapon, armor and shield that is not equipped and that the shop buys (not ammunition, throwing weapons or items marked Keep).",
        "Save often in towns.",
        "+Darklands+ also saves by itself on arriving at a town or any other place, and before a battle.",
    }, true);
    topic("The party: Endurance, Strength, Divine Favor", {
        "Endurance (End) is breath and bruises: blows take it first and rest brings it back. At 0 a member falls unconscious.",
        "Strength (Str) is real wounds: it falls with heavy blows and comes back slowly. At 0 the member is dead. Below half, keep that member out of the fight.",
        "Divine Favor (DF) is spent calling on saints and comes back with prayer (and slowly by itself).",
        "Each member's attributes and skills are on the character sheet (click the member in the party panel at the left, or press F1 ... F5).",
        "+The bars on the party panel show End, Str and DF all the time, with numbers.",
        "The party has room for four characters of your own: at an inn, \"reconsider the composition of your party\" creates a new one for an empty slot or dismisses one. A fifth slot is for companions some cards bring along for a while (a village's headman, a knight).",
        "Each member carries their own items. Only equipped items (weapons, armor, shield) count toward a member's load, and a heavy load slows them in battle.",
        "+With the shared pack (Darklands+), everything not equipped goes into one pack for the whole party, so nobody has to pass items between members. Each member keeps their equipped weapons and armor, their mount and their own potions. When the game needs a particular member to have something (lock picks at a lock, arrows before a battle, a horse), Darklands+ hands it to them from the pack first.",
        "On a card, the test is made by the leader.", "+Darklands+ has the member best at it take the test instead (F12 > Settings > Cards and towns: The best member speaks for the party), with the chance and that member's name beside the option. The leader still gets whatever a card gives \"the leader\" (a mine's traps, a church's favor), and in a battle or a dungeon, while unpaused, deals with doors, chests and locks.",
    });
    topic("Money and reputation", {
        "1 florin = 20 groschen; 1 groschen = 12 pfennigs. The purse is the party's; prices change with the town's opinion of you.",
        "The party has a separate reputation in each town: good deeds there raise it, crimes and brawls lower it. A bad reputation brings trouble with the watch and higher prices.",
        "A reputation runs from -99 to 99, in six named levels: hunted (-75 and below), wanted, suspected, unknown (-9 to 10), respected (11 to 50) and a local hero (above 50). Most deeds and saints move it by a random amount, and the further it is from 0 the likelier it stays as it is: at 50 one deed in two counts, at 90 one in ten.",
        "Fame is the party's renown everywhere; it brings a new title every 50 points.",
        "Party Info (F6, or the Party menu) shows the fame, the money and the reputation in the town the party is in; on its small map, point at a town to see the reputation there.", "+Darklands+'s Party Info lists the reputation in every town at once.",
    });
    topic("Towns", {
        "+G opens Go to: a list of the town's places (shops, churches, the inn, the gates ...); pick one and Darklands+ takes the party there, choosing the town's menu entries for you.",
        "The inn: a room for the night, or stay for days to rest, regain Strength, pray, brew, earn a little or study with a teacher.",
        "+The inn's screen shows each member's day and what it costs; \"Until rested\" and \"A week\" spend the days for you, stopping when something changes.",
        "The innkeeper keeps items for the party (store items, recover items stored here), in a store room of each town's own.",
        "+With the shared store room (Darklands+), one store room serves every inn: what is left in one town is there in all of them (F12 > Journal > Stored lists it).",
        "Teachers: a guild's masters (blacksmiths, swordsmiths, armorers, bowyers, artificers), the university, the monks, the physician and the alchemists can be asked to teach; a teacher found stays in town for a week and teaches at the inn, for a fee each day.",
        "Each shop's goods have a quality, written like 29q: most shops' weapons and armor are about 25q, the best in the land near 38q.",
        "+The right margin marks a town's notable shops (30q and better) with an icon and their quality; the world map (M) lists them all and finds where to buy a thing. In a shop, a weapon or armor is compared with what the member has equipped.",
        "Almost every town has a monastery, where the monks teach saints, pray for the party and heal. Only some towns have a university, a cathedral, an alchemist, a physician, docks with boats or a bank.",
        "+Under a town's name, the margin shows icons for what the town has that most towns lack (point at one to see what it offers); the world map (M) finds every town that has one.",
        "Night falls at 19:00 and lasts until 5:00. At night the streets have the night watch, cutthroats lie in wait for a party leaving a side street (a stealthy, streetwise party slips past them), and gates may be closed.",
        "+At night the margin shows \"Night in the streets\" with the party's chances.",
        "Witches gather at Sabbats on set nights at places in the wilds. Now and then a card gives away a Sabbat's night or place (a captured villager, a dying cultist, a witch's letter).",
        "+Once the party has heard of a Sabbat, it is listed among the quests and its place is marked on the world map (M) and in the overland view. A town that bars the party, or whose watch wants the party, has a sign beside its name on both.",
    });
    topic("Healing and rest", {
        "At an inn, a member who relaxes or regains Strength has full Endurance again the next morning. Regain Strength adds (the party's best Healing skill) / 15 Strength a day (at least 1).",
        "Praying (at an inn) adds (Religion + Virtue) / 12 + 1 Divine Favor a day.",
        "A wounded party should not travel far: an encounter on the road at low Strength can kill.",
    });
    topic("Alchemy", {
        "Alchemy is strong but slow to build up: each member knows formulae of their own, and a formula makes one potion. Alchemists sell formulae.",
        "Each town's alchemist sells four formulae, the same for the whole game, and only the formulae of the first seven potions (Noxious Aroma to Sunburst) and one Thunderbolt formula are ever sold: the others are traded for or found.",
        "+The character sheet's Formulae page, set to All formulae, names the nearest visited alchemist that sells each formula the party lacks; the margin (\"Formulae sold here\", with the prices) and the world map's tooltip on a town list what its alchemist sells.",
        "The ingredients are bought from pharmacists, herbalists and alchemists; ingredients carried by any member count.",
        "+In such a shop, the Formulae button lists the party's formulae; +1 or +5 beside one buys the ingredients for one or five more batches.",
        "A batch's chance of success: 100 + the Philosopher's Stone's quality + Intelligence + Alchemy - the formula's mystic number. The 100 is less when more batches are made at once: 50 for four or five, 33 for six or seven, and so on. The party has one Philosopher's Stone at most (Party Info, F6, shows its quality). Brewing takes a day of alchemy at an inn or in camp.",
        "A failed batch at an inn, in a village or a hideout ends in a mishap one time in two: an explosion (the party hurt, damages to pay, no room at that inn for four months) or a stink (no room for a month). The formula's risk (low, medium, high) makes it an explosion one time in four, three or two. In a camp in the wilds a failure only costs the ingredients: brew there when the chance is poor.",
        "A potion is as good as the formula it was brewed with: each potion has three formulae, of grade 1, 2 and 3.",
        "+The marks on a flask (one, two or three) show its grade (their look: F12 > Settings > Frames and clock), and the Formulae page lists every formula the party knows: who knows it, its ingredients against those carried, the batches they make and each alchemist's chance.",
    });
    topic("Saints", {
        "Saints are learned in a library (a town's monastery or university, or a monastery in the country) and called upon with Divine Favor, in a card's options or from the menu. Each saint helps with something different; calling on one takes Religion, Virtue and Divine Favor.",
        "Each library teaches four saints, the same for the whole game.",
        "+The character sheet's Saints page, set to All saints, names the libraries visited that teach each saint the party lacks, and says what each saint really does; the margin (\"Saints taught here\") and the world map say what a town's library teaches.",
        "+Darklands+ shows the chance on the prayer screen and in a card's options.",
        "A saint's help lasts about a day. Some saints ask a price for it, a loss of Perception, Intelligence, Agility or Charisma for that day (the saint's legend says so); the lost points come back when the help runs out.",
        "+The margin's \"Blessings and potions\" shows what is at work on each member, and for how long; on the character sheet's Skills page a raised value is blue.",
    });
    topic("Battle", {
        "Battles run in real time; Space pauses. To give an order, select a member (click them, or press 1 to 5), pick the order from the menus (Std Attack, Vulnerable, Berserk, Parry, Use Missile, Walk towards, Halt and more), click the enemy or the spot it is for, then press Enter.",
        "+With Easier battle controls (Darklands+), a click on an enemy (to attack it) or a spot (to walk there) gives the order at once, with no menu and no Enter, and if selecting the member paused the battle, it goes on; while the battle is paused, the next member without an order is selected. A shooter's box at the left has a bow or a sword on it: click it to choose whether a click on an enemy makes them shoot or fight hand to hand. Z fast-forwards.", "+The right margin shows the selected member's orders as buttons, so the menus are not needed.",
        "+With Improved combat (Darklands+), members fight on sensibly between orders: a shooter keeps shooting (at the nearest enemy in sight when the target falls), steps aside for a clear line, and draws a melee weapon once an enemy is close, or to help a party member nearby who is set upon by two or more or badly hurt; a member whose enemy falls turns on one close by.",
        "Ctrl+click a member and the view follows them.", "+The view scrolls when the pointer rests on its edge (the original needs Shift held there), and selecting a member does not move it: press their number twice to bring the view to them.",
        "Armor has a hardness (soft, stiff, scale, mail, plate); a weapon whose penetration is above it pierces and wounds, one equal to it is halved, one below it is mostly turned aside. Short swords and daggers do little against plate. E (Enemy Info) shows each kind of enemy.",
        "+Point at an enemy to see how your weapons fare against its armor; Enemy Info shows it for every kind of enemy on the field.",
        "Skills grow with use in battle (and with lessons at an inn); when a fight ends, each gain shows in the member's box at the left.",
        "+Darklands+ also shows each gain rising in gold over the fighter's head (+2 Edged).",
        "Missile weapons need ammunition. A shooter needs a clear line to the target: party members, enemies and corners all block it, so move the shooter where the line is clear.",
        "+Darklands+ can give the party endless ammunition (F12 > Settings > The party and its items).",
        "After a fight in a street or a dungeon, walk the party out through an exit to go on.",
        "In a castle, dungeon or mine the battle goes on after each fight; it ends when the master of the place has fallen and his fight is over, or when the party walks out (which the game counts as fleeing).",
        "+The battle margin marks the master among the enemies, and the battle pauses when a group of enemies comes out to fight. With Enemies still waiting (F12 > Mods), it also counts the enemies not yet met, floor by floor, and says on which floor the master waits.",
    });
    topic("Travel", {
        "In the overland view, clicking a spot walks the party towards it.",
        "+Ctrl+click travels there along the roads, around rivers (any key or click stops). M opens the world map: double-click a place to go there.",
        "Each step costs time, and the road brings encounters. Roads are quickest, marsh and mountains slowest. The texts of St. Christopher, St. Drogo and St. Wilfrid promise faster travel, and St. Eustace's faster travel through woods, but in the original game they do not speed the party.",
        "+The fix \"Saints do what their texts say\" (Darklands+, F12 > Settings > Fixes) makes these saints keep their texts' promise: faster travel for a week, so fewer days pass on the way.",
        "Winter is hard on the road: from November to March a blizzard can catch the party in the open (in the high mountains from October to April). Keep going, call on a saint, or camp and wait it out.",
        "Castles, monasteries, mines and the rest open their own card when the party steps on them.",
    });
    topic("Saving", {
        "Alt+S saves, with a comment, and Alt+L loads; each save is kept as a new one. Some places do not allow saving (in the middle of a card); save often in towns.",
        "+Ctrl+S quick-saves; F9 or Ctrl+L loads the quick save. On Darklands+'s Save screen, clicking an old save replaces it with the new one, and the Load screen has a Delete button.",
    });
}
