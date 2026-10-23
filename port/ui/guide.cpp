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
    ImGui::TextWrapped("Darklands has no tutorial: this page sums up how it works, with a few first steps. Nothing here tells the story; the rules are the original game's, as Darklands+ "
                       "reads them from the program. F11 lists the keys and clicks for the current screen at any time; a right-click opens the menus, with Darklands+'s own at the right end.");
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
        "A new party has little money and light armour. Paid tasks come from a town's merchants, foreign traders and pharmacists (at the marketplace), the Fugger, the Medici, the Hanse, the town council and the cathedral: robber knights to kill, items to fetch. The left margin lists the party's tasks.",
        "Armour before anything: a weapon whose penetration is below an armour's hardness is mostly deflected. Buy the hardest armour the purse allows before travelling far (Battle, below).",
        "Rest at an inn when Strength is low, and set out with it full. After dark, wait for morning at the inn rather than in the streets.",
        "Sell what nobody uses: in a shop, the Sell spare gear button sells every unused weapon, armour and shield at once.",
        "Save often in towns; Darklands+ also saves by itself on arriving in a town and before a battle.",
    }, true);
    topic("The party: Endurance, Strength, Divine Favor", {
        "Endurance (End) is breath and bruises: blows take it first and rest brings it back. At 0 a member falls unconscious.",
        "Strength (Str) is real wounds: it falls with heavy blows and comes back slowly. At 0 the member is dead. Watch the green bar: below half, keep that member out of the fight.",
        "Divine Favor (DF) is spent calling on saints and comes back with prayer (and slowly by itself).",
        "Each member's attributes and skills are on the character sheet (click the portrait, or press F1 ... F5); the bars on the party panel show End, Str and DF all the time.",
        "The party has room for four characters of your own: at an inn, \"reconsider the composition of your party\" creates a new one for an empty slot or dismisses one. A fifth slot is for companions some cards bring along for a while (a village's headman, a knight).",
        "The pack is the whole party's: a member keeps only what they use, the horse or mule they ride and their own potions; whatever else the game asks one member for (lock picks, a mount) is handed to them as it is needed. Only items in use count toward the load.",
        "On a card, each test is made by the member best at it (the chance and that member's name are beside the option). The leader still gets whatever a card gives \"the leader\" (a mine's traps, a church's favour), and in a battle or a dungeon, while unpaused, deals with doors, chests and locks and leads the group's moves.",
    });
    topic("Money and reputation", {
        "1 florin = 20 groschen; 1 groschen = 12 pfennigs. The purse is the party's; prices change with the town's opinion of you.",
        "The party has a separate reputation in each town: good deeds there raise it, crimes and brawls lower it. A bad reputation brings trouble with the watch and higher prices.",
        "A reputation runs from -99 to 99, in six named levels: hunted (-75 and below), wanted, suspected, unknown (-9 to 10), respected (11 to 50) and a local hero (above 50). Most deeds and saints move it by a random amount, and the further it is from 0 the likelier it stays as it is: at 50 one deed in two counts, at 90 one in ten.",
        "Fame is the party's renown everywhere; it brings a new title every 50 points.",
        "Party Info (F6, or the Party menu) shows the fame, the money and the reputation in every town at once.",
    });
    topic("Towns", {
        "G opens Go to: a list of the town's places (shops, churches, the inn, the gates ...) to walk to directly.",
        "The inn: a room for the night, or take up residence for days: rest, regain strength, pray, brew, earn a little or study with a teacher (the page shows each member's day and what it costs). \"Until rested\" spends the days for you.",
        "The innkeeper keeps items for the party (store items, recover items stored here): Darklands+ makes it one store room shared by every inn, so what is left in one town is there in all of them (F12 > Settings > Helpers; F12 > Journal > Stored lists it).",
        "Teachers: ask for tutoring or instruction at a monastery, university, guild, the town council or a craftsman (armourer, swordsmith, blacksmith, bowyer, artificer); those found then teach at the inn for a while, for a fee each day.",
        "Each shop's goods have a quality, written like 29q: most shops' weapons and armour are about 25q, the best in the land near 38q. The right margin names a town's notable shops (30q and better) with their pictures; F12 > Journal > Towns lists them all. Darklands+ compares a weapon or armour with what the member has now.",
        "Under a town's name, the margin shows pictures of what the town has that most towns lack: a university, a cathedral, an alchemist, a physician, docks with boats, a bank (point at one to see what it offers); the world map (M) can find every town that has one. Almost every town has a monastery, where the monks teach saints, pray for the party and heal.",
        "Night falls at 19:00 and lasts until 5:00. At night the streets have the night watch, cutthroats lie in wait for a party leaving a side street (a stealthy, streetwise party slips past them), and gates may be closed: an inn is the safe place to wait for morning. The margin then shows \"Night in the streets\" with the party's chances.",
        "Merchants, officials and others give tasks; the left margin and F12 > Journal > Quests list them with their places and time limits, each with a picture of its kind (the world map's legend explains the pictures). A town that bars the party, or whose watch wants the party, has a sign beside its name on the maps.",
        "Witches gather at Sabbats on set nights at places in the wilds. Now and then a card gives away a Sabbat's night or place (a captured villager, a dying cultist, a witch's letter); once the party has heard of a Sabbat, it is listed among the tasks and its place is marked on the maps.",
    });
    topic("Healing and rest", {
        "At an inn, a member who relaxes or regains Strength has full Endurance again the next morning. Regain Strength adds (the party's best Healing skill) / 15 Strength a day (at least 1).",
        "Praying (at an inn) adds (Religion + Virtue) / 12 + 1 Divine Favor a day.",
        "A wounded party should not travel far: an encounter on the road at low Strength can kill.",
    });
    topic("Alchemy", {
        "Each member knows formulae of their own; a formula makes one potion. Alchemists sell formulae (Darklands+ shows each formula's grade and who knows it already).",
        "Each town's alchemist sells four formulae, the same for the whole game, and only the formulae of the first seven potions (Noxious Aroma to Sunburst) and one Thunderbolt formula are ever sold: the others are traded for or found. The character sheet's Formulae page, set to All formulae, names the nearest visited alchemist that sells each formula the party lacks; the margin (\"Formulae sold here\", with the prices) and the world map's tooltip on a town list what its alchemist sells.",
        "The ingredients are bought from pharmacists, herbalists and alchemists; ingredients carried by any member count. In such a shop, the Formulae button buys the ingredients one more batch needs.",
        "A batch's chance of success: 100 + the Philosopher's Stone's quality + Intelligence + Alchemy - the formula's mystic number. The 100 is less when more batches are made at once: 50 for four or five, 33 for six or seven, and so on. The party has one Philosopher's Stone at most (Party Info, F6, says which). Brewing takes a day of Alchemy at an inn or in camp.",
        "A failed batch at an inn, in a village or a hideout ends in a mishap one time in two: an explosion (the party hurt, damages to pay, no room at that inn for four months) or a stink (no room for a month). The formula's risk (low, medium, high) makes it an explosion one time in four, three or two. In a camp in the wild a failure only costs the ingredients: brew there when the chance is poor.",
        "The Formulae page of the character sheet lists every formula the party knows: who knows it, its ingredients against those the party carries (all 23 ingredients are under the list), the batches they make and each alchemist's chance.",
        "A potion is as good as the formula it was brewed with: each potion has three formulae, of grade 1, 2 and 3, and the dots on its flask (one, two or three) show the grade. The description under a list says it too (\"grade 2 of 3\").",
    });
    topic("Saints", {
        "Saints are learned in a library (a town's monastery or university, or a monastery in the country) and called upon with Divine Favor, in a card's options or from the menu. Each saint helps with something different.",
        "Each library teaches four saints, the same for the whole game. The character sheet's Saints page, set to All saints, names the libraries the party has visited that teach each saint it lacks; the margin (\"Saints taught here\") and the world map say what a town's library teaches.",
        "Darklands+ shows the chance on the prayer page and in a card's options. The Saints page of the character sheet lists who knows which saint and what each one really does.",
        "A saint's help lasts about a day. Some saints ask a price for it, a loss of Perception, Intelligence, Agility or Charisma for that day (the saint's legend says so); the lost points come back when the help runs out. The margin's \"In force\" shows what is at work on each member, and for how long; on the character sheet's Skills page a raised value is blue.",
    });
    topic("Battle", {
        "Battles run in real time; Space pauses. To give an order, click a member (on the field, or their box at the left; a right-click on the box opens their sheet), then an enemy to attack or a spot to walk to: that click gives the order, and if selecting the member paused the battle, it goes on. While the battle is paused, the next member without an order is selected, so a battle's first orders take one click each. A member with a bow, crossbow, gun or thrown weapon has a bow or a sword on their picture at the left: click it to choose whether a click on an enemy makes them shoot or fight hand to hand. Shooters draw their melee weapon by themselves once the enemies close in, so the volleys before the lines meet need no further orders. The buttons in the right margin hold the other orders (Vulnerable, Berserk, Parry, Use Missile, Halt and more); for a member already fighting, an attack chosen there is given against that enemy at once. Z fast-forwards (Z again goes back to the chosen speed).",
        "The view scrolls when the pointer rests on its edge (or with Shift). To find a member out of sight, Ctrl+click their portrait: the view then follows them.",
        "Armour has a hardness (soft, stiff, scale, mail, plate); a weapon whose penetration is above it pierces and wounds, one equal to it is halved, one below it is mostly deflected. Point at an enemy to see this for your weapons; E (Enemy Info) shows it for every kind of enemy on the field.",
        "Skills grow with use in battle (and with lessons at an inn); the gains show on the member's box when the fight ends.",
        "Missile weapons need ammunition (Darklands+ can give the party an endless supply: F12 > Settings > Helpers). A shooter needs a clear line to the target: party members, enemies and corners all block it, so move the shooter where the line is clear. (An experimental helper in F12 > Settings > Helpers lets a shooter find a clear line by itself.)",
        "After a fight in a street or a dungeon, walk the party out through an exit to go on.",
        "In a castle, dungeon or mine the battle goes on after each fight; it ends when the master of the place has fallen and his fight is over, or when the party walks out (which the game counts as fleeing). The battle margin marks the master, lists the enemies still waiting there floor by floor, and says where the master waits; the battle pauses when a group of them comes out to fight.",
    });
    topic("Travel", {
        "On the travel map, clicking a spot walks the party towards it; Ctrl+click travels there along the roads, around rivers (any key or click stops). M opens the world map: double-click a place to go there.",
        "Each step costs time, and the road brings encounters (F12 > Settings > Helpers sets how often). Roads are quickest, marsh and mountains slowest. St. Christopher, St. Drogo and St. Wilfrid make the party travel faster for a week, St. Eustace through woods (with the fix \"Saints do what their texts say\"): fewer days pass on the way.",
        "Winter is hard on the road: from November to March a blizzard can catch the party in the open (in the high mountains from October to April). Keep going, call on a saint, or camp and wait it out.",
        "Castles, monasteries, mines and the rest open their own card when the party steps on them.",
    });
    topic("Saving", {
        "Alt+S saves with a comment, Ctrl+S quick-saves; F9 or Ctrl+L loads the quick save. Darklands+ also saves by itself on arriving in a town and before a battle (the Autosave).",
        "Every save is kept: on the Save page, clicking an old save replaces it with the new one, and the Load page has a Delete button for the chosen save.",
        "Some places do not allow saving (in the middle of a card); save often in towns.",
    });
}
