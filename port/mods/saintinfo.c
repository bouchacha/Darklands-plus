/* saintinfo.c - what each saint's help does in the game's code, beside the description the game
 * shows (DARKLAND.SNT), for Darklands+'s screens (saintinfo.h).
 *
 * A saint's help is its function in s039 165c, called through the table saint_fns (far 290e:2937)
 * with what 8 (src/c/s039/s039_165c.c; saint i is the function the table's vector i leads to, not
 * the i-th in the file). Most of it is boosts for about a day (19 to 21 hours: raise_attr_temp,
 * s029 0e76:4c04, and its twin for skills, 4b08), healing (heal_attr_pct: a part of what is lost),
 * the skills of the weapons in hand (0e76:0792: the weapon types of the melee and missile weapon
 * ready), reputation (loc_reputation_adj, 0e76:19d0: by a to b-1, with a chance of 100 less how far
 * the town's reputation is from 0) and a few events: kind 0x4b for a day, water crossed as land
 * (s030 overland); 0x4f for a week, no ambush in the meetings on the road ($MeetB01 ...); 0x5d,
 * the next step on the map leads to the nearest town. What the descriptions promise beyond that
 * (banishing demons, escaping prison, light ...) happens only where an encounter's card lets a
 * member call on the saint (card_saints: counted here per saint, the card scripts naming it).
 * Some of the help depends on the fixes (mods/fixes.c: pauls, saintrep, armourday): the texts
 * follow them. */
#include <stdio.h>
#include <string.h>
#include "fixes.h"
#include "saintinfo.h"

static const struct { const char *does, *unlike; int cards; } INFO[SAINT_INFO_N] = {
    { "For a day: Endurance +8 to 15, Charisma +6 to 11, every weapon skill +15 to 29.",
      "", 39 },   /* 0 St.Adrian */
    { "For a day: Endurance +6 to 11 (a woman: +12 to 23), Charisma +6 to 11.",
      "", 0 },   /* 1 St.Agatha */
    { "Only if a woman is in the party: may raise the reputation in the nearest town by 10 to 19; for a day, Charisma +10 to 29.",
      "", 1 },   /* 2 St.Agnes */
    { "For a day: Woodwise +25 to 29.",
      "", 10 },   /* 3 St.Aidan */
    { "For a day: Intelligence +8 to 15, Perception +6 to 11, Alchemy +30 to 44, Latin and Read & Write +15 to 29.",
      "", 0 },   /* 4 St.Albert the Great */
    { "For a day: Intelligence +6 to 11, Latin and Read & Write +20 to 39.",
      "", 5 },   /* 5 St.Alcuin */
    { "May raise the reputation in the nearest town by 10 to 19. For a day: Charisma +6 to 11. A party with neither florins nor groschen gets 3 to 9 groschen.",
      "", 1 },   /* 6 St.Alexis */
    { "For a day: Endurance +6 to 11, Charisma +8 to 15.",
      "The reputation in Scotland or Russia: places not in the game.", 0 },   /* 7 St.Andrew */
    { "For a day: Edged, Impact, Polearms, Thrown and Bows +15 to 29.",
      "", 0 },   /* 8 St.Andrew the Tribune */
    { "For a day: Endurance +4 to 9, Strength +5 to 10, Charisma +1 to 10, Speak Common and Latin +10 to 19, Read & Write +20 to 39.",
      "", 3 },   /* 9 St.Anthony */
    { "For a day: Perception +6 to 11, Strength +4 to 7, Charisma +15 to 24, Speak Common +20 to 29, Impact and Latin +10 to 19.",
      "", 1 },   /* 10 St.Anthony of Padua */
    { "For a day: Perception +6 to 11, Strength and Intelligence +4 to 7, every weapon skill +10 to 19, Streetwise and Latin +6 to 11, Read & Write and Riding +8 to 15.",
      "", 0 },   /* 11 St.Arnulf */
    { "For a day: Endurance +8 to 23, Strength +2 to 4.",
      "", 9 },   /* 12 St.Apollinarius */
    { "Artifice +15 to 29 for two hours at most; Missile Devices +20 to 39 for 5 to 7 hours.",
      "", 1 },   /* 13 St.Barbara */
    { "For a day: Intelligence +4 to 8, Perception +3 to 6.",
      "", 1 },   /* 14 St.Bathildis */
    { "For a day: Charisma +8 to 15, every weapon skill +10 to 19, Speak Common +8 to 15.",
      "", 13 },   /* 15 St.Boniface */
    { "For a day: Charisma and Intelligence +8 to 15, Speak Common +40 to 99.",
      "", 1 },   /* 16 St.Catherine */
    { "For a day: Charisma and Perception +1 to 15, Speak Common +10 to 29.",
      "", 12 },   /* 17 St.Catherine of Siena */
    { "For a day: Charisma +3 to 10. If the one prayed for carries a musical instrument, it may raise the reputation of the town the party is in (on the map: the nearest) by 20 to 59.",
      "", 7 },   /* 18 St.Cecilia */
    { "For a day: Perception and Intelligence +3 to 5, Strength +6 to 11, every weapon skill and Latin +6 to 11.",
      "", 0 },   /* 19 St.Ceolwulf */
    { "@FIRE1 May raise the reputation in the nearest town by 10 to 19. For a day: Charisma +6 to 11, Flails +15 to 29.",
      "Not in the text: Flails and the reputation. The fire protection is for the one prayed for only (the text: everyone, and one person greatly), and no Endurance is healed.", 3 },   /* 20 St.Charity */
    { "For a day: Agility +8 to 15, Charisma +1 to 10.",
      "", 8 },   /* 21 St.Christina, Astonish. */
    { "For a day: Streetwise +10 to 19, Woodwise +20 to 39, Riding +25 to 74, Bows +6 to 11.",
      "Not in the game: faster travel, and the Strength and Endurance restored.", 1 },   /* 22 St.Christopher */
    { "Heals all lost Endurance and Strength.",
      "", 3 },   /* 23 St.Clare */
    { "Heals 25% of lost Endurance and 50% of lost Strength. For a day: Charisma +6 to 11, Healing +10 to 19.",
      "", 0 },   /* 24 St.Clotilda */
    { "For a day: Charisma +8 to 15, Speak Common +30 to 89, Latin +6 to 11, Read & Write +10 to 19.",
      "", 0 },   /* 25 St.Colman of Cloyne */
    { "For a day: Endurance +8 to 15, Strength +6 to 13, Charisma +4 to 7, Perception -5 to 9, the skills of the weapons in hand +10 to 19.",
      "", 0 },   /* 26 St.Columba */
    { "Heals 10% of lost Endurance and 10% of lost Strength. For a day: Perception +4 to 7, Healing +40 to 79. Teaches St. Damian.",
      "", 0 },   /* 27 St.Cosmas */
    { "@CRISPIN",
      "The text says non-metal armour; the game raises all armour.", 0 },   /* 28 St.Crispin */
    { "For a day: Intelligence +5 to 9, Alchemy +5 to 9.",
      "", 3 },   /* 29 St.Cyprian */
    { "Heals all lost Endurance and 30% of lost Strength. Teaches St. Cosmas.",
      "", 0 },   /* 30 St.Damian */
    { "For a day: Intelligence +4 to 7, Charisma +1 to 6, Perception +6 to 11, the skills of the weapons in hand +8 to 15, Riding +6 to 11.",
      "", 0 },   /* 31 St.David */
    { "Heals all lost Strength. For a day: Charisma +15 to 44, Speak Common +25 to 49.",
      "The reputation in Paris and France: places not in the game.", 0 },   /* 32 St.Denis */
    { "Heals 30% of lost Endurance. For a day: Strength +5 to 14, the skills of the weapons in hand +20 to 59.",
      "", 0 },   /* 33 St.Derfel Gadarn */
    { "Heals all lost Endurance and 20% of lost Strength. For a day: Charisma +15 to 29.",
      "The reputation in Corsica and Monaco: places not in the game.", 4 },   /* 34 St.Devota */
    { "For a day: Agility +10 to 19, Woodwise and Stealth +10 to 19, Streetwise and Artifice +20 to 39.",
      "Only the one prayed for, not the entire party; no Charisma or Speak Common.", 6 },   /* 35 St.Dismas */
    { "For a day: Intelligence +10 to 19, Speak Common and Latin +10 to 19, Read & Write +20 to 39.",
      "", 3 },   /* 36 St.Dominic */
    { "For a day: Charisma +10 to 19, Perception +6 to 11, Healing +10 to 19. A member with no edged weapon gets a long sword.",
      "", 11 },   /* 37 St.Dorothy of Montau */
    { "For a day: Perception +6 to 11, Healing +10 to 19, Woodwise +20 to 39. For a week no ambush takes the party by surprise (bandits and the like).",
      "Not in the game: faster travel. Instead, a week safe from ambushes.", 1 },   /* 38 St.Drogo */
    { "For a day: Perception +6 to 11, the skills of the weapons in hand +6 to 11, Artifice +25 to 74, Streetwise +6 to 11, Alchemy +5 to 14. @ARM1",
      "The text says the quality of all metal armour in the party; the game gives the one prayed for armour one level thicker.", 0 },   /* 39 St.Dunstan */
    { "Heals 30% of lost Endurance. For a day: Agility +4 to 7, Stealth +15 to 29, Healing +10 to 19.",
      "", 2 },   /* 40 St.Dymphna */
    { "For a day: Endurance +4 to 7, Intelligence +6 to 11, Perception +8 to 15, Riding +8 to 15, every weapon skill +6 to 11. Every member's Virtue is raised to at least 20, for good, and the party loses a quarter of its money.",
      "", 6 },   /* 41 St.Edward, Confessor */
    { "For a day: Artifice +25 to 74, Alchemy +8 to 15.",
      "", 0 },   /* 42 St.Eligius */
    { "For a day: Charisma +15 to 29, Speak Common +20 to 39.",
      "", 9 },   /* 43 St.Emydius */
    { "For a day: Charisma +5 to 9, Riding and Woodwise +10 to 19, Speak Common +6 to 11. For a day the party can cross water as if it were land.",
      "", 1 },   /* 44 St.Engelbert */
    { "@FIRE1 For a day: Agility and Perception +6 to 11.",
      "", 2 },   /* 45 St.Erasmus */
    { "For a day: Strength +8 to 23, Charisma +4 to 7, the skills of the weapons in hand +15 to 24, Riding and Speak Common +6 to 11.",
      "The reputation in Sweden: a place not in the game.", 6 },   /* 46 St.Eric */
    { "For a day: Riding +10 to 19, Woodwise +40 to 69, Edged, Thrown and Bows +6 to 11.",
      "Not in the game: faster travel through woods. The weapon skills are only Edged, Thrown and Bows.", 2 },   /* 47 St.Eustace */
    { "For a day: Agility +8 to 15, Stealth +40 to 119, Woodwise +10 to 19.",
      "", 2 },   /* 48 St.Felix of Nola */
    { "For a day: Charisma +8 to 15, Perception +6 to 11, Latin +10 to 19.",
      "", 2 },   /* 49 St.Finbar */
    { "For a day: Agility +8 to 15, Charisma +6 to 11, Streetwise +4 to 7, Speak Common and Read & Write +6 to 11. For a day the party can cross water as if it were land.",
      "", 3 },   /* 50 St.Finnian */
    { "May raise the reputation by 15 to 24 in Passau, Salzburg, Linz, Steyr, Wien or Graz (nowhere else). For a day: the skills of the weapons in hand +6 to 11, Riding +6 to 11. For a day the party can cross water as if it were land.",
      "", 3 },   /* 51 St.Florian */
    { "Heals 30% of lost Endurance and 10% of lost Strength. For a day: Perception and Endurance +6 to 11, Charisma +15 to 29, Speak Common and Woodwise +4 to 7, Healing +10 to 19, Artifice +12 to 23.",
      "Not in the game: the Virtue and the loss of cash. Instead Healing, Artifice and Woodwise.", 1 },   /* 52 St.Francis */
    { "For a day: Intelligence and Perception +10 to 29.",
      "", 14 },   /* 53 St.Gabriel */
    { "Heals 30% of lost Endurance and 20% of lost Strength. For a week no ambush takes the party by surprise (bandits and the like).",
      "", 22 },   /* 54 St.Genevieve */
    { "For a day: Strength +10 to 14, Agility +5 to 9, every weapon skill and Riding +25 to 49.",
      "", 0 },   /* 55 St.George */
    { "Heals 30% of lost Endurance and 20% of lost Strength. For a day: the skills of the weapons in hand +15 to 24, Healing +20 to 39.",
      "", 0 },   /* 56 St.Gerlac */
    { "For a day: Charisma +6 to 11, Riding +5 to 9, Woodwise, Latin and Read & Write +15 to 29. At the party's next step on the map it is carried to the nearest town.",
      "", 4 },   /* 57 St.Gertrude o'Nivelles */
    { "Heals 40% of lost Endurance and 10% of lost Strength. For a day: Agility -4 to 7, the skills of the weapons in hand -15 to 24, Stealth +30 to 59, Woodwise +5 to 9, Streetwise +50 to 89.",
      "Woodwise and Streetwise the other way round (the text: a little Streetwise, a tremendous Woodwise); no money.", 0 },   /* 58 St.Giles */
    { "For a day: Intelligence +5 to 9, Perception +15 to 24, Alchemy +40 to 59, Artifice +10 to 19. Costs a point of Strength (a wound) and a point of Virtue, for good.",
      "", 0 },   /* 59 St.Giles of Portugal */
    { "Heals 30% of lost Endurance. For a day: Latin +20 to 39, Read & Write +25 to 49.",
      "", 4 },   /* 60 St.Godehard */
    { "Heals 20% of lost Endurance. For a day: Speak Common +15 to 29, Artifice +10 to 19. For a week no ambush takes the party by surprise (bandits and the like).",
      "", 30 },   /* 61 St.Godfrey */
    { "May raise the reputation by 20 to 39 in Schleswig, Lübeck, Wismar, Rostock, Hamburg, Lüneburg or Bremen (nowhere else). For a day: every weapon skill +15 to 29.",
      "", 0 },   /* 62 St.Gottschalk */
    { "For a day: Charisma +8 to 15, Alchemy and Speak Common +10 to 19, Artifice +20 to 39.",
      "", 13 },   /* 63 St.Gregory Thaumaturg. */
    { "May raise the reputation by 30 to 49 in Frankfurt an der Oder, Breslau, Olmütz or Teschen (nowhere else). For a day: Healing +20 to 29.",
      "", 6 },   /* 64 St.Hedwig */
    { "For a day: Intelligence +8 to 23, every weapon skill +10 to 19.",
      "No reputation in its help.", 1 },   /* 65 St.Henry */
    { "For a day: Charisma +10 to 19, Strength +4 to 7.",
      "", 8 },   /* 66 St.Heribert */
    { "For a day: Perception +15 to 29.",
      "", 10 },   /* 67 St.Herve */
    { "For a day: Perception +20 to 59.",
      "No reputation in its help.", 10 },   /* 68 St.Hildegard */
    { "For a day: Polearms, Thrown and Bows +15 to 29, Missile Devices +10 to 19, Stealth +25 to 49, Woodwise +40 to 79.",
      "", 11 },   /* 69 St.Hubert */
    { "For a day: Charisma +15 to 29, every weapon skill and Speak Common +10 to 19, Riding +5 to 9.",
      "", 0 },   /* 70 St.Illtyd */
    { "For a day: Charisma +8 to 15, Streetwise +10 to 19, Speak Common +5 to 9.",
      "The reputation in Madrid and Spain: places not in the game.", 2 },   /* 71 St.Isidore */
    { "Heals 50% of lost Endurance and all lost Strength. For a day: Charisma +8 to 15, Healing +20 to 39.",
      "", 0 },   /* 72 St.Ita */
    { "Heals 20% of lost Endurance and 10% of lost Strength. For a day: Strength and Endurance +4 to 7, Charisma +3 to 5, Virtue +20 to 34.",
      "The reputation in Spain: a place not in the game; no Intelligence lost.", 0 },   /* 73 St.James */
    { "No help of its own: its powers work only where an encounter calls for it.",
      "", 3 },   /* 74 St.Januarius */
    { "A woman: heals all lost Endurance and 20% of lost Strength, and for a day Endurance +5 to 9, Strength +4 to 7. A man: heals 30% of lost Endurance and 10% of lost Strength.",
      "", 0 },   /* 75 St.John of Bridlington */
    { "For a day: Charisma +8 to 15, Perception -6 to 11, Speak Common and Latin +30 to 59.",
      "", 0 },   /* 76 St.John Chrysostom */
    { "For a day: Alchemy, Religion, Speak Common, Latin, Read & Write, Healing, Artifice, Stealth, Streetwise and Woodwise each +1 to 4, and 1 more for every 5 points of Virtue.",
      "", 0 },   /* 77 St.John Climacus */
    { "@NEPOMUK For a day: Charisma +8 to 15, Intelligence +6 to 11, Speak Common +20 to 39, Streetwise +4 to 7.",
      "", 6 },   /* 78 St.John Nepolmuchen */
    { "For a day: Charisma +6 to 11, Artifice and Streetwise +30 to 59.",
      "", 1 },   /* 79 St.Joseph */
    { "For a day: Endurance, Charisma, Strength, Perception, Agility and Intelligence +4 to 8, the skills of the weapons in hand +15 to 24, Alchemy, Religion, Speak Common, Latin, Read & Write, Healing, Stealth, Artifice, Woodwise and Streetwise +6 to 11.",
      "", 9 },   /* 80 St.Jude */
    { "For a day: Woodwise +20 to 39, Riding +6 to 11. For a day the party can cross water as if it were land.",
      "", 1 },   /* 81 St.Julian, Hospitaler */
    { "Heals 40% of lost Endurance and 20% of lost Strength. For a day: Charisma +6 to 11, Speak Common +15 to 29, Riding +4 to 7, Healing +10 to 19.",
      "", 0 },   /* 82 St.Kessog */
    { "For a day: Strength +4 to 7, Agility +3 to 5, the skills of the weapons in hand +15 to 24, Speak Common +6 to 11, Riding +10 to 19.",
      "", 0 },   /* 83 St.Lasdislaus */
    { "Heals all lost Endurance. For a day: Charisma +12 to 19, Speak Common and Streetwise +10 to 19.",
      "", 4 },   /* 84 St.Lawrence */
    { "Heals 30% of lost Endurance and all lost Strength.",
      "", 1 },   /* 85 St.Lazarus */
    { "For a day: every weapon skill +10 to 19.",
      "", 0 },   /* 86 St.Longinius */
    { "Heals all lost Endurance and Strength. For a day: Perception +6 to 11.",
      "", 3 },   /* 87 St.Lucy */
    { "Heals 50% of lost Endurance and 30% of lost Strength. For a day: Healing +20 to 39.",
      "", 0 },   /* 88 St.Luke */
    { "For a day: Charisma +6 to 11, Perception +8 to 15, Endurance +5 to 9, Virtue +6 to 17.",
      "Endurance is raised, not spent.", 53 },   /* 89 St.Lutgardis */
    { "A woman: heals all lost Endurance and half the lost Strength, and for a day every attribute +3 to 6 and Alchemy, Religion, Speak Common, Latin, Read & Write, Healing, Artifice, Stealth, Streetwise and Woodwise +5 to 10. A man: heals 10% of lost Endurance and Strength.",
      "Not in the game: the protection from flames.", 3 },   /* 90 St.Margaret */
    { "Heals all lost Endurance and 40% of lost Strength. For a day: Healing +20 to 39, Speak Common +10 to 19.",
      "", 0 },   /* 91 St.Margaret of Cortona */
    { "For a day: Agility +12 to 23, Read & Write +15 to 29.",
      "", 0 },   /* 92 St.Mark */
    { "Heals 30% of lost Endurance and 30% of lost Strength. For a day: Charisma +8 to 15, Perception +12 to 19, Speak Common +25 to 49, Riding +20 to 39.",
      "", 10 },   /* 93 St.Martin of Tours */
    { "For a day: Intelligence +8 to 15, Latin +15 to 29, Read & Write +25 to 49. The party's standing with the Fugger and with the Medici rises by 20 to 39 each.",
      "", 0 },   /* 94 St.Matthew */
    { "For a day: Edged +20 to 39, Alchemy +10 to 19.",
      "", 0 },   /* 95 St.Maurice */
    { "May raise the reputation in the nearest town by 20 to 39. For a day: Charisma +12 to 19, the skills of the weapons in hand +20, Healing +20 to 39.",
      "In combat it raises the skills of the weapons in hand, not the weapons and armour.", 0 },   /* 96 St.Michael */
    { "For a day: Charisma +10 to 19, Healing +20 to 39. For a day the party can cross water as if it were land.",
      "", 53 },   /* 97 St.Milburga */
    { "For a day: every weapon skill +15 to 29, Stealth +20 to 39, Woodwise and Streetwise +10 to 19.",
      "", 1 },   /* 98 St.Moses the Black */
    { "May raise the reputation in the nearest town by 10 to 19. For a day: Charisma +6 to 11, Endurance +4 to 8, Flails +15 to 29.",
      "Flails and Endurance instead of Agility and Riding; the reputation anywhere, not only in his lands.", 2 },   /* 99 St.Nicholas */
    { "May raise the reputation by 15 to 24 in Strassburg or Basel (nowhere else). For a day: Perception +6 to 11.",
      "", 2 },   /* 100 St.Odilia */
    { "For a day: Perception +10 to 19, Charisma +4 to 8, every weapon skill +8 to 22, Speak Common +8 to 15.",
      "", 0 },   /* 101 St.Odo */
    { "Heals 30% of lost Endurance. For a day: every weapon skill +8 to 22.",
      "", 0 },   /* 102 St.Olaf */
    { "@FIRE1 Heals 30% of lost Endurance and 30% of lost Strength. For a day: Charisma +6 to 11, Alchemy +10 to 19, Healing +15 to 29. For a day the party can cross water as if it were land.",
      "Fire protection for the one prayed for (not the party), and the crossing of water, not in the text; nothing against animals.", 5 },   /* 103 St.Pantaleon */
    { "For a day: Charisma +15 to 24, Strength +12 to 19, Edged, Impact, Polearms, Speak Common, Latin and Read & Write +15 to 29. @ARM2",
      "The armour is against every blow, not only missiles.", 0 },   /* 104 St.Patrick */
    { "@PAUL",
      "The reputation in Greece and Malta: places not in the game.", 0 },   /* 105 St.Paul */
    { "@SIMPLE",
      "", 4 },   /* 106 St.Paul the Simple */
    { "For a day: Charisma +15 to 29.",
      "", 7 },   /* 107 St.Perpetua */
    { "For a day: Charisma +8 to 15, Strength +12 to 19, Perception lowered by half its maximum, Speak Common, Latin and Healing +10 to 19, Edged +15 to 29.",
      "", 5 },   /* 108 St.Peter */
    { "May lower the reputation in the nearest town by 1 to 4. For a day: Intelligence and Perception +6 to 11, Alchemy and Riding +10 to 19, Stealth +40 to 79.",
      "", 0 },   /* 109 St.Peter of Atroa */
    { "@POLYCARP",
      "", 3 },   /* 110 St.Polycarp */
    { "Heals all lost Endurance and Strength. For a day: Agility +8 to 15, Perception +6 to 11, Healing +50 to 139.",
      "", 14 },   /* 111 St.Raphael */
    { "For a day: Intelligence +15 to 29, Read & Write +30 to 89. For a day the party can cross water as if it were land.",
      "No reputation with the Dominicans in its help.", 5 },   /* 112 St.Raymond Penafort */
    { "For a day: Intelligence +12 to 19, Alchemy +20 to 39, Read & Write +30 to 59. Endurance and Strength below 9 are raised to 9.",
      "", 0 },   /* 113 St.Raymond Lull */
    { "Heals 30% of lost Endurance. For a day: Charisma -6 to 11, Alchemy +6 to 17, Artifice +35 to 69.",
      "", 7 },   /* 114 St.Reinold */
    { "Heals 20% of lost Endurance and 20% of lost Strength. For a day: Healing +10 to 19, Artifice +8 to 15.",
      "", 2 },   /* 115 St.Roch */
    { "Heals all lost Endurance and Strength. For a day: Charisma +10 to 19.",
      "", 0 },   /* 116 St.Sabas the Goth */
    { "Heals 10% of lost Endurance and 10% of lost Strength. For a day: Agility +12 to 19, Bows +20 to 39, Riding +10 to 19.",
      "", 2 },   /* 117 St.Sebastian */
    { "Heals 20% of lost Endurance and 10% of lost Strength. For a day: Charisma +6 to 11, Speak Common +30 to 59, Latin +20 to 39.",
      "No reputation in its help (Poland and Cracow: no such places in the game).", 0 },   /* 118 St.Stanislaus */
    { "In Pressburg, may raise the reputation by 15 to 24. For a day: Charisma +6 to 11, Intelligence +8 to 15, the skills of the weapons in hand +15 to 24, Speak Common +6 to 11, Riding +10 to 19.",
      "", 0 },   /* 119 St.Stephen */
    { "Heals 20% of lost Endurance and 10% of lost Strength. For a day: Charisma +10 to 19, Speak Common +25 to 49.",
      "", 0 },   /* 120 St.Swithbert */
    { "Heals 40% of lost Endurance and 20% of lost Strength. For a day: Charisma +6 to 11.",
      "", 7 },   /* 121 St.Tarachus */
    { "Heals 40% of lost Endurance and 20% of lost Strength. For a day: Healing +30 to 59.",
      "", 0 },   /* 122 St.Thalelaeus,Merciful */
    { "For a day: Charisma +6 to 11, Endurance +4 to 8, the skills of the weapons in hand +15 to 24, Flails +15 to 29.",
      "Flails instead of Riding.", 0 },   /* 123 St.Theodore Tiro */
    { "For a day: Strength and Endurance +4 to 8, Speak Common and Woodwise +4 to 7, Healing +10 to 19, Artifice +12 to 23.",
      "", 0 },   /* 124 St.Thomas the Apostle */
    { "For a day: Intelligence +12 to 23, Speak Common and Latin +12 to 26, Read & Write +15 to 29, Healing +10 to 19.",
      "Not in the game: Religion and the reputation at monasteries and universities. Instead Speak Common and Healing.", 3 },   /* 125 St.Thomas Aquinas */
    { "Heals 10% of lost Endurance and 10% of lost Strength. For a day: Charisma +10 to 19, Stealth, Healing and Speak Common +6 to 11.",
      "", 3 },   /* 126 St.Valentine */
    { "Heals all lost Endurance and 20% of lost Strength. For a day: Charisma +8 to 15, the skills of the weapons in hand +15 to 24, Speak Common +10 to 19.",
      "", 0 },   /* 127 St.Victor o'Marseilles */
    { "For a day: Charisma +10 to 19, Agility +12 to 23. At the party's next step on the map it is carried to the nearest town.",
      "No Endurance lost.", 40 },   /* 128 St.Vitus */
    { "May raise the reputation by 15 to 24 in St. Joachimsthal, Bürglitz, Prag, Kuttenberg, Olmütz or Brünn (nowhere else). For a day: Perception and Strength +6 to 11, Intelligence +8 to 13, the skills of the weapons in hand +8 to 15, Riding +8 to 15.",
      "No reputation with nobles in its help.", 3 },   /* 129 St.Wenceslaus */
    { "Heals 20% of lost Endurance. For a day: Charisma +6 to 11, Artifice +6 to 11, Speak Common +20 to 39. For a week no ambush takes the party by surprise (bandits and the like).",
      "Not in the game: faster travel. Instead, a week safe from ambushes.", 2 },   /* 130 St.Wilfrid */
    { "For a day: Agility +8 to 15, Woodwise +8 to 15, Riding +10 to 19. For a week no ambush takes the party by surprise (bandits and the like).",
      "", 10 },   /* 131 St.Willehad */
    { "For a day: Charisma +10 to 19, Woodwise +6 to 11, Speak Common +25 to 49.",
      "", 0 },   /* 132 St.Willebald */
    { "For a day: Charisma +10 to 19, Woodwise +8 to 15, Speak Common +20 to 39.",
      "No reputation in its help.", 0 },   /* 133 St.Williborord */
    { "For a day: Charisma +8 to 15, Intelligence +6 to 11, Woodwise and Riding +6 to 11, Speak Common +20 to 39.",
      "No reputation in its help.", 2 },   /* 134 St.Wolfgang */
    { "Heals 20% of lost Endurance and 5% of lost Strength. For a day: Perception +6 to 11, Streetwise +6 to 11.",
      "", 2 },   /* 135 St.Zita */
};

/* the help that depends on a fix */
static const char *special(int s, const char *tok, char *buf, size_t n)
{
    int arm = fix_is_on("armourday"), fire = fix_is_on("fireward"), pauls = fix_is_on("pauls"), nep = fix_is_on("saintrep");
    (void)s;
    if (!strcmp(tok, "FIRE1")) return arm ? "Fire protection 90 for a day, for the one prayed for." :
        fire ? "Meant as protection against fire: written where the battle reads armour, for good (it does nothing with the fire protection fix)."
             : "Meant as protection against fire: armour +32 for good, for the one prayed for (where the battle reads armour).";
    if (!strcmp(tok, "CRISPIN")) return arm ? "Every member's worn armour counts 10 points better in quality, for a day."
                                            : "Every member's worn body and leg armour +10 quality, for good, piling up with each prayer.";
    if (!strcmp(tok, "ARM1")) return arm ? "Armour one level thicker, for a day." : "Armour one level thicker, for good, piling up.";
    if (!strcmp(tok, "ARM2")) return arm ? "Armour two levels thicker, for a day." : "Armour two levels thicker, for good, piling up.";
    if (!strcmp(tok, "NEPOMUK")) return nep ? "May raise the reputation by 15 to 24 in Prag, and by 10 to 19 in St. Joachimsthal, Bürglitz, Kuttenberg, Olmütz or Brünn (nowhere else)."
        : "May raise the reputation by 10 to 19 in St. Joachimsthal, Bürglitz, Kuttenberg, Olmütz or Brünn (nowhere else); in Prag it goes to a place picked at random.";
    if (!strcmp(tok, "PAUL")) return pauls ? "Heals 10% of lost Strength. For a day: Endurance +5 to 9, Charisma +8 to 15, Speak Common +20 to 39, Latin +10 to 19, Read & Write +40 to 79." : "Heals all lost Endurance and Strength. For a day: Intelligence lowered by half its maximum.";
    if (!strcmp(tok, "SIMPLE")) return pauls ? "Heals all lost Endurance and Strength. For a day: Intelligence lowered by half its maximum." : "For a day: Intelligence +12 to 23, Speak Common and Latin +12 to 26, Read & Write +15 to 29, Healing +10 to 19.";
    if (!strcmp(tok, "POLYCARP")) return arm ? "Every member: fire protection 90 for a day." :
        fire ? "Every member: fire protection 90 for good (and armour bits the fire protection fix hides)."
             : "Every member: fire protection 90 for good, and armour +48 for good (meant as protection against fire).";
    snprintf(buf, n, "?");
    return buf;
}

const char *saint_info_does(int s)
{
    static char out[600];
    if (s < 0 || s >= SAINT_INFO_N) return "";
    const char *t = INFO[s].does;
    if (!strchr(t, '@')) return t;
    size_t o = 0;                                   /* (@TOKEN: the part a fix decides) */
    for (const char *p = t; *p && o + 1 < sizeof out; ) {
        if (*p != '@') { out[o++] = *p++; continue; }
        char tok[16]; size_t k = 0; p++;
        while ((*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9')) { if (k + 1 < sizeof tok) tok[k++] = *p; p++; }
        tok[k] = 0;
        char b[64]; const char *r = special(s, tok, b, sizeof b);
        size_t l = strlen(r); if (o + l >= sizeof out) l = sizeof out - 1 - o;
        memcpy(out + o, r, l); o += l;
    }
    out[o] = 0;
    return out;
}

const char *saint_info_unlike(int s)
{
    if (s < 0 || s >= SAINT_INFO_N) return "";
    if (s == 105 && !fix_is_on("pauls")) return "This is the help the texts and the manual give St. Paul the Simple: the two Pauls are mixed up (the \"pauls\" fix).";
    if (s == 106 && !fix_is_on("pauls")) return "This is St. Thomas Aquinas's help: the two Pauls are mixed up (the \"pauls\" fix).";
    return INFO[s].unlike;
}

int saint_info_cards(int s) { return s >= 0 && s < SAINT_INFO_N ? INFO[s].cards : 0; }
