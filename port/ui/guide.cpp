// guide.cpp - F12 > Guide: how Darklands works, in short, for a new player (the game was sold
// with a thick manual and does not explain itself). Only the game's own rules as Darklands+
// reads them from the program (endurance and strength, the inn's numbers, alchemy's chance,
// armour against penetration ...), no story; each topic says which Darklands+ page shows more.
#include "imgui.h"
#include <string.h>
#include <initializer_list>

#include "../runtime/config.h"
void ui_guide_tab(void)
{
    if (!config_get_int("guide.seen", 0)) { config_set_int("guide.seen", 1); config_save(); }     /* (the margin's hint is not needed any more) */
    ImGui::TextWrapped("Darklands has no tutorial: this is the short of it, with a few first steps. Nothing here tells the story; the rules are the game's own, as Darklands+ "
                       "reads them from the program. F1 at any moment lists the keys and clicks of that moment; a right click opens the menus, Darklands+'s own at the right end.");
    ImGui::Spacing();
    auto topic = [](const char *title, std::initializer_list<const char *> lines, bool open = false) {
        if (ImGui::CollapsingHeader(title, open ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
            ImGui::Indent();
            for (const char *l : lines) { ImGui::Bullet(); ImGui::TextWrapped("%s", l); }
            ImGui::Unindent();
            ImGui::Spacing();
        }
    };
    topic("First steps", {
        "A new party has little money and light armour. Paid tasks come from a town's merchants, foreign traders and pharmacists (at the marketplace), the Fugger, the Medici, the Hanse, the town council and the cathedral: robber knights to kill, things to fetch. The left margin keeps each task.",
        "Armour before anything: a weapon whose penetration is below an armour's hardness is mostly deflected. Buy the hardest armour the purse allows before travelling far (Battle, below).",
        "Rest at an inn when strength is low, and set out with it full. After dark, wait for morning at the inn rather than in the streets.",
        "Sell what nobody uses: in a shop, Sell spare gear sells every unused weapon, armour and shield at once.",
        "Save often in towns; Darklands+ also saves by itself on arriving in a town and before a battle.",
    }, true);
    topic("The party: endurance, strength, divine favor", {
        "Endurance (End) is breath and bruises: blows take it first and rest brings it back. At 0 a member falls unconscious.",
        "Strength (Str) is real wounds: it falls with heavy blows and comes back slowly. At 0 the member is dead. Watch the green bar: below half, keep that member out of the fight.",
        "Divine Favor (DF) is spent calling on saints and comes back with prayer (and slowly by itself).",
        "Each member's attributes and skills are on the character sheet (a click on the portrait, or F1 ... F5); the bars on the party panel show End, Str and DF all the time.",
        "The party has four places for characters of your own: at an inn, \"reconsider the composition of your party\" makes a new one for an empty place or lets one go. A fifth place is for companions some cards bring along for a while (a village's headman, a knight).",
        "The pack is the whole party's: a member keeps only what they use, the horse or mule they ride and their own potions; whatever else the game asks one member for (lock picks, a mount) is handed to them as it is needed. Only what is in use weighs.",
        "A card's tests are made by the member best at each (the chance and the name are beside the option). The leader still takes what befalls \"the leader\" on a card (a mine's traps, a church's favour), and in a battle or a dungeon, unpaused, deals with doors, chests and locks and leads the group's moves.",
    });
    topic("Money and reputation", {
        "1 florin = 20 groschen; 1 groschen = 12 pfennigs. The purse is the party's; prices change with the town's opinion of you.",
        "Each town has its own reputation for you: good deeds there raise it, crimes and brawls lower it. A bad name brings trouble with the watch and higher prices.",
        "A reputation runs from -99 to 99, in six words: hunted (-75 and below), wanted, suspected, unknown (-9 to 10), respected (11 to 50) and a local hero (above 50). Most deeds and saints move it by a random amount, and the further it is from 0 the likelier it stays as it is: at 50 one deed in two counts, at 90 one in ten.",
        "Fame is the party's renown everywhere; it brings a new title every 50 points.",
        "Party Info (F6, or the Party menu) shows the fame, the money and the reputation in every town at once.",
    });
    topic("Towns", {
        "G opens Go to: a list of the town's places (shops, churches, the inn, the gates ...) to walk to directly.",
        "The inn: a room for the night, or take up residence for days: rest, regain strength, pray, brew, earn a little or study with a teacher (the page shows each member's day and what it costs). \"Until rested\" spends the days for you.",
        "The innkeeper keeps things for the party (store items, recover items stored here): Darklands+ makes it one store room for every inn, so what is left in one town is there in all (F12 > Settings > Helpers; F12 > Journal > Stored lists it).",
        "Teachers: ask for tutoring or instruction at a monastery, university, guild, the town council or a craftsman (armourer, swordsmith, blacksmith, bowyer, artificer); those found then teach at the inn for a while, for a fee each day.",
        "Shops sell by quality, written like 29q: most shops' weapons and armour are about 25q, the best in the land near 38q. The right margin names a town's notable shops (30q and better) with their pictures; F12 > Journal > Towns lists them all. Darklands+ compares a weapon or armour with what the member has now.",
        "Under a town's name the margin shows, as pictures, what it has that most towns lack: a university, a cathedral, an alchemist, a physician, docks with boats, a bank (point at one to see what it offers); the world map (M) finds every town with one. Almost every town has a monastery, where the monks teach saints, pray for the party and heal.",
        "Night falls at 19:00 and lasts until 5:00. The streets then have the night watch and, leaving a side street, cutthroats (a stealthy, streetwise party slips past them), and gates may be closed: an inn is the safe place to wait for morning. The margin says \"Night in the streets\" with the party's chances.",
        "Merchants, officials and others give tasks; the left margin and F12 > Journal > Quests keep them with their places and time limits, each with a picture of its kind (the world map's legend lists them). A town that bars the party, or whose watch wants it, has a sign beside its name on the maps.",
        "Witches gather at Sabbats on set nights at places in the wilds. Cards let slip a Sabbat's night or place now and then (a captured villager, a dying cultist, a witch's letter); once the party has heard of one, it is among the tasks, its place marked on the maps.",
    });
    topic("Healing and rest", {
        "At an inn, a member who relaxes or regains strength has full endurance again the next morning. Regain Strength adds the party's best Healing skill / 15 (at least 1) a day.",
        "Praying (at an inn) adds (Religion + Virtue) / 12 + 1 divine favor a day.",
        "A wounded party should not travel far: an encounter on the road at low strength can kill.",
    });
    topic("Alchemy", {
        "Each member knows formulae of their own; a formula makes one potion. Alchemists sell formulae (Darklands+ shows each one's tier and who knows it already).",
        "Each town's alchemist sells four formulae, the same for the whole game, and only the formulae of the first seven potions (Noxious Aroma to Sunburst) and one of Thunderbolt are ever sold: the others are traded for or found. The character sheet's Formulae (All formulae) name the nearest alchemist the party has been to that sells each formula it lacks; the margin (\"Formulae sold here\", with the prices) and the world map's tooltip on a town list what its alchemist sells.",
        "The ingredients are bought from pharmacists, herbalists and alchemists; the whole party's carried ingredients count. In such a shop, the Formulae button buys what a batch more needs.",
        "The chance of a batch: 100 + the Philosopher's Stone's quality + Intelligence + Alchemy - the formula's mystic number. The 100 is less when more batches are made at once: 50 for four or five, 33 for six or seven, and so on. The party has one Philosopher's Stone at most (Party Info, F6, says which). Mixing takes a day: Alchemy work at an inn or in camp.",
        "A failed batch at an inn, in a village or a hideout goes wrong one time in two: an explosion (the party hurt, damages to pay, no room at that inn for four months) or a stink (no room for a month). The formula's risk (low, medium, high) makes it an explosion one time in four, three or two. In a camp in the wild a failure only costs the ingredients: brew there when the chance is poor.",
        "The character sheet's Formulae list every formula the party knows: who knows it, its ingredients against the party's (all 23 ingredients under the list), the batches they make and each alchemist's chance.",
        "A potion is as good as the formula that brewed it: each potion has three formulae, of grade 1, 2 and 3, and the dots on its flask (one, two or three) tell which. The description under a list says it too (\"grade 2 of 3\").",
    });
    topic("Saints", {
        "Saints are learned in a library (a town's monastery or university, or a monastery in the country) and called upon with divine favor, in a card's options or from the menu. Each helps with something of their own.",
        "Each library teaches four saints, the same for the whole game. The character sheet's Saints (All saints) say which libraries the party has been to teach a saint it lacks; the margin (\"Saints taught here\") and the world map say what a town's library teaches.",
        "Darklands+ shows the chance on the prayer page and in a card's options. The character sheet's Saints list who knows which saint and what each really does.",
        "A saint's help lasts about a day. Some saints ask a price for it, a loss of Perception, Intelligence, Agility or Charisma for that day (their legend says so); it comes back when the help runs out. The margin's \"In force\" shows what is at work on each member, and for how long; on the character sheet's Skills a raised value is blue.",
    });
    topic("Battle", {
        "Battles run in real time; Space pauses, and orders can be given while paused: click a member, then an enemy (attack) or a place (walk). Z fast-forwards (and Z again slows down).",
        "The view scrolls when the pointer rests on its edge (or with Shift). A member out of sight: Ctrl + click their portrait, and the view follows them.",
        "Armour has a hardness (soft, stiff, scale, mail, plate); a weapon's penetration above it pierces and wounds, equal to it is halved, below it is mostly deflected. Point at an enemy to see this for your weapons; E (Enemy Info) shows it for every kind of foe on the field.",
        "Skills grow with use in battle (and with lessons at an inn); the gains show on the member's box when the fight ends.",
        "Missile weapons need ammunition (Darklands+ gives the party's an endless supply, F12 > Settings > Helpers). A shooter needs a clear line to the target: friends, foes and corners all block it, so move the shooter where the line is clear. (An experimental helper in F12 > Settings > Helpers lets a shooter look for a line by itself.)",
        "After a fight in a street or a dungeon, walk the party out through an exit to go on.",
        "In a castle, dungeon or mine the battle goes on after each fight; it ends when the master of the place has fallen and his fight is over, or when the party walks out (which the game counts as fleeing). The battle margin marks the master, lists the foes still waiting there floor by floor, and says where the master waits; the battle pauses when a group of them comes out to fight.",
    });
    topic("Travel", {
        "On the map, a click walks towards a spot; Ctrl + click travels there along the roads, around rivers (any key or click stops). M opens the world map: double-click a place to go there.",
        "Each step costs time, and the road brings encounters (F12 > Settings > Helpers sets how often). Roads are quickest, marsh and mountains slowest. St. Christopher, St. Drogo and St. Wilfrid make the party travel faster for a week, St. Eustace through woods (with the fix \"Saints do what their texts say\"): fewer days pass on the way.",
        "Winter is hard on the road: from November to March a blizzard can catch the party in the open (in the high mountains from October to April). Keep going, call on a saint, or camp and wait it out.",
        "Castles, monasteries, mines and the rest open their own card when the party steps on them.",
    });
    topic("Saving", {
        "Alt+S saves with a comment, Ctrl+S quick-saves; F9 or Ctrl+L loads the quick save. Darklands+ also saves by itself on arriving in a town and before a battle (the Autosave).",
        "The game keeps every save: on the Save page a click on an old save makes the new one take its place, and the Load page deletes the save chosen.",
        "Some places do not allow saving (in the middle of a card); save often in towns.",
    });
}
