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
    ImGui::TextWrapped("Darklands has no tutorial: this is the short of it. Nothing here tells the story; it is the game's own rules, as Darklands+ "
                       "reads them from the program. F1 at any moment lists the keys and clicks of that moment.");
    ImGui::Spacing();
    auto topic = [](const char *title, std::initializer_list<const char *> lines, bool open = false) {
        if (ImGui::CollapsingHeader(title, open ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
            ImGui::Indent();
            for (const char *l : lines) { ImGui::Bullet(); ImGui::TextWrapped("%s", l); }
            ImGui::Unindent();
            ImGui::Spacing();
        }
    };
    topic("The party: endurance, strength, divine favor", {
        "Endurance (End) is breath and bruises: blows take it first and rest brings it back. At 0 a member falls unconscious.",
        "Strength (Str) is real wounds: it falls with heavy blows and comes back slowly. At 0 the member is dead. Watch the green bar: below half, keep that member out of the fight.",
        "Divine Favor (DF) is spent calling on saints and comes back with prayer (and slowly by itself).",
        "Each member's attributes and skills are on the character sheet (a click on the portrait, or F1 ... F5); the bars on the party panel show End, Str and DF all the time.",
        "The pack is the whole party's: a member keeps only what they use and the horse or mule they ride, and whatever the game asks one member for (a potion in a battle or on a card, lock picks, arrows, a mount) is handed to them as it is needed. Only what is in use weighs.",
        "A card's tests are made by the member best at each (the chance and the name are beside the option). The leader still takes what befalls \"the leader\" on a card, and acts first in a battle.",
    }, true);
    topic("Money and reputation", {
        "1 florin = 20 groschen; 1 groschen = 12 pfennigs. The purse is the party's; prices change with the town's opinion of you.",
        "Each town has its own reputation for you: good deeds there raise it, crimes and brawls lower it. A bad name brings trouble with the watch and higher prices.",
        "Fame is the party's renown everywhere; it brings a new title every 50 points.",
        "Party Info (F6, or the Party menu) shows the fame, the money and the reputation in every town at once.",
    });
    topic("Towns", {
        "G opens Go to: a list of the town's places (shops, churches, the inn, the gates ...) to walk to directly.",
        "The inn: a room for the night, or take up residence for days: rest, regain strength, pray, brew, earn a little or study with a teacher (the page shows each member's day and what it costs). \"Until rested\" spends the days for you.",
        "Teachers: ask for tutoring or instruction at a monastery, university, guild, the town council or a craftsman (armourer, swordsmith, blacksmith, bowyer, artificer); those found then teach at the inn for a while, for a fee each day.",
        "Shops sell by quality: the higher a shop's rating, the better its wares (the right margin lists every shop of the town with its rating). Darklands+ compares a weapon or armour with what the member has now.",
        "Streets at night are dangerous, and gates may be closed: an inn is the safe place to wait for morning.",
        "Merchants, officials and others give tasks; the left margin and F12 > Journal > Quests keep them with their places and time limits.",
    });
    topic("Healing and rest", {
        "At an inn, a member who relaxes or regains strength has full endurance again the next morning. Regain Strength adds the party's best Healing skill / 15 (at least 1) a day.",
        "Praying (at an inn) adds (Religion + Virtue) / 12 + 1 divine favor a day.",
        "A wounded party should not travel far: an encounter on the road at low strength can kill.",
    });
    topic("Alchemy", {
        "Each member knows formulae of their own; a formula makes one potion. Alchemists sell formulae (Darklands+ shows each one's tier and who knows it already).",
        "The ingredients are bought from pharmacists, herbalists and alchemists; the whole party's carried ingredients count. In such a shop, the Formulae button buys what a batch more needs.",
        "The chance of a batch: 100 + Intelligence + Alchemy - the formula's mystic number, lower when more batches are made at once. Mixing takes a day: Alchemy work at an inn or in camp.",
        "F12 > Alchemy lists every formula the party knows with its ingredients and chance.",
    });
    topic("Saints", {
        "Saints are learned (at monasteries, universities and cathedrals, among others) and called upon with divine favor, in a card's options or from the menu. Each helps with something of their own.",
        "Darklands+ shows the chance on the prayer page and in a card's options. F12 > Saints lists who knows which saint and what each does.",
    });
    topic("Battle", {
        "Battles run in real time; Space pauses, and orders can be given while paused: click a member, then an enemy (attack) or a place (walk). Z fast-forwards (and Z again slows down).",
        "Armour has a hardness (soft, stiff, scale, mail, plate); a weapon's penetration above it pierces and wounds, equal to it is halved, below it is mostly deflected. Point at an enemy to see this for your weapons; E (Enemy Info) shows it for every kind of foe on the field.",
        "Skills grow with use in battle (and with lessons at an inn); the gains show on the member's box when the fight ends.",
        "Missile weapons need ammunition (Darklands+ gives the party's an endless supply, F12 > Helpers).",
        "After a fight in a street or a dungeon, walk the party out through an exit to go on.",
    });
    topic("Travel", {
        "On the map, a click walks towards a spot; Ctrl + click travels there along the roads, around rivers (any key or click stops). M opens the world map: double-click a place to go there.",
        "Each step costs time, and the road brings encounters (F12 > Helpers sets how often).",
        "Castles, monasteries, mines and the rest open their own card when the party steps on them.",
    });
    topic("Saving", {
        "Alt+S saves with a comment, Ctrl+S quick-saves; F9 or Ctrl+L loads the quick save. Darklands+ also saves by itself on arriving in a town and before a battle (the Autosave).",
        "Some places do not allow saving (in the middle of a card); save often in towns.",
    });
}
