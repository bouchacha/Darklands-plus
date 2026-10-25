// guide.cpp - F12 > New Player Guide: how Darklands works, in short, for a new player (the game was sold
// with a thick manual and does not explain itself). The game's own rules as Darklands+ reads them
// from the program (Endurance and Strength, the inn's numbers, alchemy's chance, armor against
// penetration ...), and the advice players have long given one another (the GOG and Steam forums,
// the hint book), no story. A line that starts with '+' depends on something Darklands+ adds:
// it is drawn in its own color, so a player of the original can tell it apart.
#include "imgui.h"
#include <string.h>
#include <string>
#include <vector>
#include <initializer_list>
#include "ui.h"
#include "uiinternal.h"
#include "ornament.h"

#include "../runtime/config.h"

static const ImVec4 PLUS_INK(0.10f, 0.30f, 0.58f, 1);     /* what Darklands+ adds (the legend says so) */

/* its topics are short, so they are listed at the left as Settings' sections are (a click brings one to
   the top; the one in view is marked); the weapons and the enemies, long tables, are tabs of their own */
void ui_guide_tab(void)
{
    if (!config_get_int("guide.seen", 0)) { config_set_int("guide.seen", 1); config_save(); }     /* (the margin's hint is not needed any more) */
    static std::vector<std::string> titles; static int jump = -1, at_top = 0;
    ImGui::BeginChild("##gnav", ImVec2(UIS(230), 0), ImGuiChildFlags_Borders);
    for (int i = 0; i < (int)titles.size(); i++) {
        std::string t = titles[i].substr(0, titles[i].find(':'));               /* ("The party: Endurance, ..." as "The party") */
        char lab[96]; snprintf(lab, sizeof lab, "%s###gn%d", t.c_str(), i);
        if (ImGui::Selectable(lab, i == at_top)) jump = i;
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("##gbody", ImVec2(0, 0));
    std::vector<std::string> now; int top_found = -1; float view_top = ImGui::GetWindowPos().y + UIS(30);
    ImGui::TextWrapped("Darklands has no tutorial: this page sums up how it works, with the advice players have long given one another. "
                       "F11 (a Darklands+ addition) lists the keys and clicks of the current screen at any time; a right-click opens the game's menus.");
    ImGui::PushStyleColor(ImGuiCol_Text, PLUS_INK);
    ImGui::TextWrapped("Lines in this color depend on something Darklands+ adds (most can be switched off in F12 > Settings).");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    auto topic = [&](const char *title, std::initializer_list<const char *> lines, bool = false) {
        int i = (int)now.size(); now.push_back(title);
        ImGui::Spacing();
        ImGui::PushFont(ui_font_head); orn_septext(title); ImGui::PopFont();
        if (jump == i) { ImGui::SetScrollHereY(0.0f); jump = -1; }
        if (ImGui::GetItemRectMin().y <= view_top) top_found = i;    /* (the last heading above the top: the topic in view) */
        ImGui::Indent();
        for (const char *l : lines) {
            bool plus = l[0] == '+';
            ImGui::Bullet();
            if (plus) ImGui::PushStyleColor(ImGuiCol_Text, PLUS_INK);
            ImGui::TextWrapped("%s", plus ? l + 1 : l);
            if (plus) ImGui::PopStyleColor();
        }
        ImGui::Unindent();
    };
    topic("First steps", {
        "A new party starts with 200 florins and whatever armor each member wore in their last career. Buy the best armor the members can carry before anything else (mail if you can): a blow whose penetration is below the armor's hardness seldom wounds (Battle, below).",
        "The usual way to toughen a new party: walk a town's side streets at night. Cutthroats lie in wait for a party leaving a side street (a stealthy, streetwise party slips past them); each fight raises the weapon skills used, and their weapons and armor bring a little money (clothing none: no shop buys it). The cutthroats come stronger and more numerous as the party grows, and losing to them costs the party its money and belongings and a little of its name in that town. Go back to the inn to rest between rounds.",
        "The night watch stops a party in the streets at night. Owning up costs a fine (more for a bigger party and a fuller purse). Fighting the watch costs a great deal of the town's opinion and turns the town hostile for days, and losing to them means prison and the loss of the party's arms, armor and valuables.",
        "+Darklands+ can make these night ambushes rarer (F12 > Mods: Night encounters in the streets); leave it as the original if you mean to train this way.",
        "Quests (paid work) come from a town's merchants, foreign traders and pharmacists (at the marketplace), the Fugger, the Medici, the Hanse, the town council and the cathedral: robber knights to kill, items to fetch. The town council pays best; the Fugger, the Medici and the Hanse pay about a third as much. The original game keeps no record of them: players kept pen and paper at hand.",
        "+Darklands+ keeps the record: a quest log. The left margin lists the party's quests, and the world map (M) lists them too and marks where each one leads.",
        "Rest when Strength is low, and set out with it full: at an inn, or in camp in the wilds, where healing and prayer work the same.",
        "Camping costs no money, but each day in camp makes it likelier that bandits or a lord's huntsmen find it: post a guard (Woodwise and Stealth count, and a weapon skill of 30 or more) and move on after a few days. A guard does not rest that day; a camp found with no guard is taken by surprise.",
        "Keep the party's Virtue up: a saint helps less the less Virtue the caller has. Fighting a friar on the road costs every member Virtue and Divine Favor, win or lose. Cleared streets and good deeds raise the town's opinion of the party.",
        "A good party has a leader with high Charisma and Speak Common, someone with Speak Latin (a skill of its own, apart from Read & Write), and a woodsman (Woodwise) for the road.",
        "+Sell what nobody uses: in a shop, the Sell spare gear button sells at once every weapon, armor and shield that is not equipped and brings a price (not ammunition, throwing weapons or items marked Keep).",
        "Save often: the game saves anywhere but in a fight and on the shop, character sheet, inn or camp, loot and party screens.",
        "+Darklands+ also saves by itself on arriving at a town or any other place, and before a battle.",
    }, true);
    topic("The party: Endurance, Strength, Divine Favor", {
        "Endurance (End) is breath and bruises: every blow takes some. A day's rest brings it back, and outside a battle much of it comes back by itself hour by hour. At 0 a member falls unconscious.",
        "Strength (Str) is real wounds: a blow takes some of it too, more when it pierces the armor, and it comes back slowly. At 0 the member is dead. Below half, keep that member out of the fight.",
        "Divine Favor (DF) is spent calling on saints and comes back with prayer (and slowly by itself).",
        "Each member's attributes and skills are on the character sheet (click the member in the party panel at the left, or press F1 ... F5).",
        "+The bars on the party panel show End, Str and DF all the time, with numbers.",
        "The party has five places: up to four characters of your own (at an inn, \"reconsider the composition of your party\" creates a new one for an empty place or dismisses one), and companions some cards bring along for a while (a village's headman, a knight), each taking a free place.",
        "Each member carries their own items. Only equipped items (weapons, armor, shield) count toward a member's load, and a heavy load slows them in battle.",
        "+With the shared pack (Darklands+), everything not equipped goes into one pack for the whole party, so nobody has to pass items between members. Each member keeps their equipped weapons and armor, their mount and their own potions. When the game needs a particular member to have something (lock picks at a lock, arrows before a battle, a horse), Darklands+ hands it to them from the pack first.",
        "On a card, the leader speaks for the party; many other tests count the party's best member at it, or its weakest.", "+Where the leader would take a test, Darklands+ has the member best at it take it instead (F12 > Settings > Cards and towns: The best member speaks for the party), with the chance and that member's head beside the option; the arrow at the end of the chance shows every member's, and a click on one has that member speak instead. Whoever speaks also bears what comes of it, as the card's text says: wounds, a trap's lasting harm, a shrine's favor. On the alchemist's formula options you pick who speaks (each member with their own chance), since what is traded is learned by the one who speaks. In a battle or a dungeon, while unpaused, the leader deals with doors, chests and locks.",
    });
    topic("Money and reputation", {
        "1 florin = 20 groschen; 1 groschen = 12 pfennigs. The purse is the party's. A shop's prices depend on the town's size, the leader's Charisma (each point above or below 25 is 1% better or worse) and a bad name in the town.",
        "The party has a separate reputation in each town: good deeds there raise it, crimes and brawls lower it. A bad one (-2 and below) makes prices up to 9% worse; a good one gives no discount. At -75 and below the town is hostile.",
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
        "Each shop's goods have a quality, written like 29q: most shops' weapons and armor are about 25q, the best weapon and armor shops in the land near 38q.",
        "+The right margin marks a town's notable shops (30q and better) with an icon and their quality; the world map (M) lists them all and finds where to buy a thing. In a shop, a weapon or armor is compared with what the member has equipped.",
        "Almost every town has a monastery (where the monks teach saints, pray for the party and heal), a physician, and the Fugger's, the Medici's and the Hanse's offices at the marketplace; most have an alchemist and docks. A cathedral is in about half the towns, a university in thirteen.",
        "+Under a town's name, the margin shows icons for what the town has that most towns lack (point at one to see what it offers); the world map (M) finds every town that has one.",
        "Night falls at 19:00 and lasts until 5:00. At night the streets have the night watch, cutthroats lie in wait for a party leaving a side street (a stealthy, streetwise party slips past them), and gates may be closed.",
        "+At night the margin shows \"Night in the streets\" with the party's chances.",
        "Witches gather at Sabbats on set nights at places in the wilds. Now and then a card gives away a Sabbat's night or place (a dying cultist, a witch's lover, a hidden note).",
        "+Once the party has heard of a Sabbat, it is listed among the quests and its place is marked on the world map (M) and in the overland view. A town that bars the party, or whose watch wants the party, has a sign beside its name on both.",
    });
    topic("Healing and rest", {
        "At an inn or in camp, a member who relaxes or regains Strength has full Endurance again the next morning. Regain Strength adds (the party's best Healing skill) / 15 Strength a day (at least 1).",
        "Praying (at an inn or in camp) adds (Religion + Virtue) / 12 + 1 Divine Favor a day.",
        "A wounded party should not travel far: an encounter on the road at low Strength can kill.",
    });
    topic("Alchemy", {
        "Alchemy is strong but slow to build up: each member knows formulae of their own, and a formula makes one potion. Alchemists sell formulae.",
        "Each town's alchemist sells four formulae, the same for the whole game, and only the formulae of the first seven potions (Noxious Aroma to Sunburst) and one Thunderbolt formula are ever sold: the others are traded for or found.",
        "+The character sheet's Formulae page, set to All formulae, names the nearest visited alchemist that sells each formula the party lacks; the margin (\"Formulae sold here\", with the prices) and the world map's tooltip on a town list what its alchemist sells.",
        "The ingredients are sold by a marketplace's pharmacists, by universities and at some mines, and a physician or an alchemist will trade in them; ingredients carried by any member count.",
        "+In such a shop, the Formulae button lists the party's formulae; +1 or +5 beside one buys the ingredients for one or five more batches.",
        "A batch's chance of success: 100 + the Philosopher's Stone's quality + Intelligence + Alchemy - the formula's mystic number. The 100 is less when more batches are made at once: 50 for four or five, 33 for six or seven, and so on. The party has one Philosopher's Stone at most (Party Info, F6, shows its quality). Brewing takes a day of alchemy at an inn or in camp.",
        "A failed batch at an inn, in a village or a hideout ends in a mishap one time in two: an explosion (the party hurt, damages to pay, no room at that inn for four months) or a stink (no room for a month). Which of the two it is goes by the formula's risk (low, medium, high), but backwards: an explosion three times in four at low risk, two in three at medium, one in two at high. In a camp in the wilds a failure only costs the ingredients: brew there when the chance is poor.",
        "+The fix \"The risk levels of alchemy formulae work the right way around\" (F12 > Settings > Fixes) makes it one time in four at low risk, one in three at medium and one in two at high.",
        "A potion is as good as the formula it was brewed with: each potion has three formulae, of grade 1, 2 and 3.",
        "+The marks on a flask (one, two or three) show its grade (their look: F12 > Settings > Frames and clock), and the Formulae page lists every formula the party knows: who knows it, its ingredients against those carried, the batches they make and each alchemist's chance.",
    });
    topic("Saints", {
        "Saints are learned in a library (a town's monastery or university, or a monastery in the country) and called upon with Divine Favor, in a card's options or from the menu. Each saint helps with something different; the chance goes by the caller's Virtue and the Divine Favor spent, against what that saint asks for.",
        "Each library teaches four saints, the same for the whole game: one saint to one member a visit, for a small gift.",
        "+The character sheet's Saints page, set to All saints, names the libraries visited that teach each saint the party lacks, and says what each saint really does; the margin (\"Saints taught here\") and the world map say what a town's library teaches.",
        "+Darklands+ shows the chance on the prayer screen and in a card's options.",
        "Most saints' help lasts about a day; healing and a better name in a town stay, and St. Drogo's and St. Wilfrid's shelter from ambushes lasts a week. Some saints ask a price for it, a loss of Perception, Intelligence, Agility or Charisma for that day (the saint's legend says so), which comes back when the help runs out; St. Edward the Confessor takes a quarter of the party's money.",
        "+The margin's \"Blessings and potions\" shows what is at work on each member, and for how long; on the character sheet's Skills page a raised value is blue.",
    });
    topic("Battle", {
        "Battles run in real time; Space pauses. To give an order, select a member (click them, or press 1 to 5), pick the order from the menus (Std Attack, Vulnerable, Berserk, Parry, Use Missile, Walk towards, Halt and more), click the enemy or the spot it is for, then press Enter.",
        "+With Easier battle controls (Darklands+), a click on an enemy (to attack it) or a spot (to walk there) gives the order at once, with no menu and no Enter, and if selecting the member paused the battle, it goes on; while the battle is paused, the next member without an order is selected. A shooter's box at the left has a bow or a sword on it: click it to choose whether a click on an enemy makes them shoot or fight hand to hand. Z fast-forwards.", "+The right margin shows the selected member's orders as buttons, so the menus are not needed.",
        "+With Improved combat (Darklands+), members fight on sensibly between orders: a shooter keeps shooting (at the nearest enemy in sight when the target falls), steps aside for a clear line, and draws a melee weapon once an enemy is close, or to help a party member nearby who is set upon by two or more or badly hurt; a member whose enemy falls turns on one close by.",
        "Ctrl+click a member and the view follows them.", "+The view scrolls when the pointer rests on its edge (the original needs Shift held there), and selecting a member does not move it: press their number twice to bring the view to them.",
        "Armor has a hardness (soft, stiff, scale, mail, plate) and a weapon a penetration. Above the hardness, a blow does its full damage and much of it wounds (Strength); equal to it, the weapon's extra damage is halved and less of it wounds; below it, that extra is cut to an eighth and the blow seldom wounds. Plate turns every sword, dagger and axe unless the blow is aimed at the gaps (Vulnerable: 1 to 4 more penetration); the great hammer, the long and composite bows, crossbows and the brass handgun pierce it. E (Enemy Info) shows the first two kinds of enemy on the field.",
        "+Point at an enemy to see how your weapons fare against its armor; Enemy Info shows it for every kind of enemy on the field.",
        "Skills grow with use in battle (and with lessons at an inn); when a fight ends, each gain shows in the member's box at the left.",
        "+Darklands+ also shows each gain rising in gold over the fighter's head (+2 Edged).",
        "Missile weapons need ammunition. A shooter needs a clear line to the target: party members, enemies and corners all block it, so move the shooter where the line is clear.",
        "+Darklands+ can give the party endless ammunition (F12 > Settings > The party and its items).",
        "After a fight in a street or a dungeon, walk the party out through an exit to go on.",
        "In a castle, dungeon or mine the battle goes on after each fight; it ends when the master of the place has fallen and his fight is over. Walking out while enemies are at the party is fleeing; between fights it brings the party back to the place's card.",
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
        "Alt+S saves, with a comment, and Alt+L loads; each save is kept as a new one. Saving works anywhere but in a fight (some places allow it between fights: \"Battlefield save rules are in effect\") and on the shop, character sheet, inn or camp, loot and party screens.",
        "+Ctrl+S quick-saves; F9 or Ctrl+L loads the quick save. On Darklands+'s Save screen, clicking an old save replaces it with the new one, and the Load screen has a Delete button.",
        "+Ironman (F12 > Settings > Preset, for the next new game): the game keeps one save and saves over it as you go (every card, each arrival, before each battle, on quitting). There is no loading back, and when the party's journey ends its save is deleted.",
    });
    titles = now; at_top = top_found < 0 ? 0 : top_found;
    ImGui::EndChild();
}
