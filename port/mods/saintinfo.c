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
 * Some of the help depends on the fixes (mods/fixes.c: pauls, saintrep, armourday; sainttexts,
 * mods/sainttexts.c, gives 39 saints the help their texts describe): the texts follow them. A
 * boost lasts about a day: the texts say so only of other spans (saint_info_note, beside a help that wears off). */
#include <stdio.h>
#include <string.h>
#include "fixes.h"
#include "saintinfo.h"

/* does / unlike: the game's code; st_does / st_unlike: with the sainttexts fix (0: the same), st_arm:
   that needs armourday (else the game's help); cards: the encounters that call for the saint */
static const struct { const char *does, *unlike, *st_does, *st_unlike; int st_arm, cards; } INFO[SAINT_INFO_N] = {
    { "Endurance +8 to 15, Charisma +6 to 11, every weapon skill +15 to 29.",
      "",
      "Heals 8 to 15 Endurance (up to its maximum). Charisma +6 to 11, every weapon skill +15 to 29.", "", 0, 39 },   /* 0 St.Adrian */
    { "Endurance +6 to 11 (a woman: +12 to 23), Charisma +6 to 11.",
      "",
      "Heals 6 to 11 Endurance (a woman: 12 to 23; up to its maximum). Charisma +6 to 11.", "", 0, 0 },   /* 1 St.Agatha */
    { "Only if a woman is in the party: may raise the reputation in the nearest town by 10 to 19; Charisma +10 to 29.",
      "",
      0, 0, 0, 1 },   /* 2 St.Agnes */
    { "Woodwise +25 to 29.",
      "",
      0, 0, 0, 10 },   /* 3 St.Aidan */
    { "Intelligence +8 to 15, Perception +6 to 11, Alchemy +30 to 44, Latin and Read & Write +15 to 29.",
      "",
      0, 0, 0, 0 },   /* 4 St.Albert the Great */
    { "Intelligence +6 to 11, Latin and Read & Write +20 to 39.",
      "",
      0, 0, 0, 5 },   /* 5 St.Alcuin */
    { "May raise the reputation in the nearest town by 10 to 19. Charisma +6 to 11. A party with neither florins nor groschen gets 3 to 9 groschen.",
      "",
      0, 0, 0, 1 },   /* 6 St.Alexis */
    { "Endurance +6 to 11, Charisma +8 to 15.",
      "The reputation in Scotland or Russia: places not in the game.",
      0, 0, 0, 0 },   /* 7 St.Andrew */
    { "Edged, Impact, Polearms, Thrown and Bows +15 to 29.",
      "",
      0, 0, 0, 0 },   /* 8 St.Andrew the Tribune */
    { "Endurance +4 to 9, Strength +5 to 10, Charisma +1 to 10, Speak Common and Latin +10 to 19, Read & Write +20 to 39.",
      "",
      "Heals 4 to 9 Endurance (up to its maximum). Strength +5 to 10, Charisma +1 to 10, Speak Common and Latin +10 to 19, Read & Write +20 to 39.", "", 0, 3 },   /* 9 St.Anthony */
    { "Perception +6 to 11, Strength +4 to 7, Charisma +15 to 24, Speak Common +20 to 29, Impact and Latin +10 to 19.",
      "",
      0, 0, 0, 1 },   /* 10 St.Anthony of Padua */
    { "Perception +6 to 11, Strength and Intelligence +4 to 7, every weapon skill +10 to 19, Streetwise and Latin +6 to 11, Read & Write and Riding +8 to 15.",
      "",
      0, 0, 0, 0 },   /* 11 St.Arnulf */
    { "Endurance +8 to 23, Strength +2 to 4.",
      "",
      "Heals 30% of lost Endurance and 10% of lost Strength.", "", 0, 9 },   /* 12 St.Apollinarius */
    { "Artifice +15 to 29 for two hours at most; Missile Devices +20 to 39 for 5 to 7 hours.",
      "",
      "Artifice +15 to 29 for two hours at most; Missile Devices +20 to 39 for 5 to 7 hours. In a battle, wounds cannot bring the one prayed for below 1 Strength: no death from wounds.", "", 0, 1 },   /* 13 St.Barbara */
    { "Intelligence +4 to 8, Perception +3 to 6.",
      "",
      0, 0, 0, 1 },   /* 14 St.Bathildis */
    { "Charisma +8 to 15, every weapon skill +10 to 19, Speak Common +8 to 15.",
      "",
      0, 0, 0, 13 },   /* 15 St.Boniface */
    { "Charisma and Intelligence +8 to 15, Speak Common +40 to 99.",
      "",
      0, 0, 0, 1 },   /* 16 St.Catherine */
    { "Charisma and Perception +1 to 15, Speak Common +10 to 29.",
      "",
      0, 0, 0, 12 },   /* 17 St.Catherine of Siena */
    { "Charisma +3 to 10. If the one prayed for carries a musical instrument, it may raise the reputation of the town the party is in (on the map: the nearest) by 20 to 59.",
      "",
      0, 0, 0, 7 },   /* 18 St.Cecilia */
    { "Perception and Intelligence +3 to 5, Strength +6 to 11, every weapon skill and Latin +6 to 11.",
      "",
      0, 0, 0, 0 },   /* 19 St.Ceolwulf */
    { "@FIRE1 May raise the reputation in the nearest town by 10 to 19. Charisma +6 to 11, Flails +15 to 29.",
      "Not in the text: Flails and the reputation. The fire protection is for the one prayed for only (the text: everyone, and one person greatly), and no Endurance is healed.",
      "Fire protection 50 for every member, 90 for the one prayed for. Heals 10% of lost Endurance.", "", 1, 3 },   /* 20 St.Charity */
    { "Agility +8 to 15, Charisma +1 to 10.",
      "",
      0, 0, 0, 8 },   /* 21 St.Christina, Astonish. */
    { "Streetwise +10 to 19, Woodwise +20 to 39, Riding +25 to 74, Bows +6 to 11.",
      "Not in the game: faster travel, and the Strength and Endurance restored.",
      "The party travels half again as fast for a week (fewer days on the road). Heals 10% of lost Strength and 20% of lost Endurance. Streetwise +10 to 19, Woodwise +20 to 39, Riding +25 to 74, Bows +6 to 11.", "", 0, 1 },   /* 22 St.Christopher */
    { "Heals all lost Endurance and Strength.",
      "",
      0, 0, 0, 3 },   /* 23 St.Clare */
    { "Heals 25% of lost Endurance and 50% of lost Strength. Charisma +6 to 11, Healing +10 to 19.",
      "",
      0, 0, 0, 0 },   /* 24 St.Clotilda */
    { "Charisma +8 to 15, Speak Common +30 to 89, Latin +6 to 11, Read & Write +10 to 19.",
      "",
      0, 0, 0, 0 },   /* 25 St.Colman of Cloyne */
    { "Endurance +8 to 15, Strength +6 to 13, Charisma +4 to 7, Perception lowered by 5 to 9, the skills of the weapons in hand +10 to 19.",
      "",
      0, 0, 0, 0 },   /* 26 St.Columba */
    { "Heals 10% of lost Endurance and 10% of lost Strength. Perception +4 to 7, Healing +40 to 79. Teaches St. Damian.",
      "",
      0, 0, 0, 0 },   /* 27 St.Cosmas */
    { "@CRISPIN",
      "The text says non-metal armour; the game raises all armour.",
      "Every member's leather and cloth armour counts 10 points better in quality.", "", 1, 0 },   /* 28 St.Crispin */
    { "Intelligence +5 to 9, Alchemy +5 to 9.",
      "",
      0, 0, 0, 3 },   /* 29 St.Cyprian */
    { "Heals all lost Endurance and 30% of lost Strength. Teaches St. Cosmas.",
      "",
      0, 0, 0, 0 },   /* 30 St.Damian */
    { "Intelligence +4 to 7, Charisma +1 to 6, Perception +6 to 11, the skills of the weapons in hand +8 to 15, Riding +6 to 11.",
      "",
      0, 0, 0, 0 },   /* 31 St.David */
    { "Heals all lost Strength. Charisma +15 to 44, Speak Common +25 to 49.",
      "The reputation in Paris and France: places not in the game.",
      "Heals all lost Strength and 10% of lost Endurance. Charisma +15 to 44, Speak Common +25 to 49.", "The reputation in Paris and France: places not in the game.", 0, 0 },   /* 32 St.Denis */
    { "Heals 30% of lost Endurance. Strength +5 to 14, the skills of the weapons in hand +20 to 59.",
      "",
      0, 0, 0, 0 },   /* 33 St.Derfel Gadarn */
    { "Heals all lost Endurance and 20% of lost Strength. Charisma +15 to 29.",
      "The reputation in Corsica and Monaco: places not in the game.",
      0, 0, 0, 4 },   /* 34 St.Devota */
    { "Agility +10 to 19, Woodwise and Stealth +10 to 19, Streetwise and Artifice +20 to 39.",
      "Only the one prayed for, not the entire party; no Charisma or Speak Common.",
      "Every member's Stealth +10 to 19. Agility, Woodwise +10 to 19, Streetwise and Artifice +20 to 39, Charisma and Speak Common +6 to 11.", "", 0, 6 },   /* 35 St.Dismas */
    { "Intelligence +10 to 19, Speak Common and Latin +10 to 19, Read & Write +20 to 39.",
      "",
      0, 0, 0, 3 },   /* 36 St.Dominic */
    { "Charisma +10 to 19, Perception +6 to 11, Healing +10 to 19. A member with no edged weapon gets a long sword.",
      "",
      0, 0, 0, 11 },   /* 37 St.Dorothy of Montau */
    { "Perception +6 to 11, Healing +10 to 19, Woodwise +20 to 39. For a week no ambush takes the party by surprise (bandits and the like).",
      "Not in the game: faster travel. Instead, a week safe from ambushes.",
      "The party travels half again as fast for a week. Perception +6 to 11, Healing +10 to 19, Woodwise +20 to 39.", "", 0, 1 },   /* 38 St.Drogo */
    { "Perception +6 to 11, the skills of the weapons in hand +6 to 11, Artifice +25 to 74, Streetwise +6 to 11, Alchemy +5 to 14. @ARM1",
      "The text says the quality of all metal armour in the party; the game gives the one prayed for armour one level thicker.",
      "Every member's metal armour (scale, mail, plate) counts 10 points better in quality. Perception +6 to 11, the skills of the weapons in hand +6 to 11, Artifice +25 to 74, Streetwise +6 to 11, Alchemy +5 to 14.", "", 1, 0 },   /* 39 St.Dunstan */
    { "Heals 30% of lost Endurance. Agility +4 to 7, Stealth +15 to 29, Healing +10 to 19.",
      "",
      0, 0, 0, 2 },   /* 40 St.Dymphna */
    { "Endurance +4 to 7, Intelligence +6 to 11, Perception +8 to 15, Riding +8 to 15, every weapon skill +6 to 11. Every member's Virtue is raised to at least 20, for good, and the party loses a quarter of its money.",
      "",
      0, 0, 0, 6 },   /* 41 St.Edward, Confessor */
    { "Artifice +25 to 74, Alchemy +8 to 15.",
      "",
      "Blades, hammers and maces, polearms, crossbows and guns of the one prayed for count 10 points better in quality. Artifice +25 to 74, Alchemy +8 to 15.", "", 0, 0 },   /* 42 St.Eligius */
    { "Charisma +15 to 29, Speak Common +20 to 39.",
      "",
      0, 0, 0, 9 },   /* 43 St.Emydius */
    { "Charisma +5 to 9, Riding and Woodwise +10 to 19, Speak Common +6 to 11. The party can cross water as if it were land.",
      "",
      0, 0, 0, 1 },   /* 44 St.Engelbert */
    { "@FIRE1 Agility and Perception +6 to 11.",
      "",
      "Fire protection 50 for every member. Agility and Perception +6 to 11.", "", 1, 2 },   /* 45 St.Erasmus */
    { "Strength +8 to 23, Charisma +4 to 7, the skills of the weapons in hand +15 to 24, Riding and Speak Common +6 to 11.",
      "The reputation in Sweden: a place not in the game.",
      0, 0, 0, 6 },   /* 46 St.Eric */
    { "Riding +10 to 19, Woodwise +40 to 69, Edged, Thrown and Bows +6 to 11.",
      "Not in the game: faster travel through woods. The weapon skills are only Edged, Thrown and Bows.",
      "The party travels twice as fast through woods and forests for a week. Riding +10 to 19, Woodwise +40 to 69, Edged, Polearms, Thrown, Bows and Missile Devices +15 to 29.", "", 0, 2 },   /* 47 St.Eustace */
    { "Agility +8 to 15, Stealth +40 to 119, Woodwise +10 to 19.",
      "",
      0, 0, 0, 2 },   /* 48 St.Felix of Nola */
    { "Charisma +8 to 15, Perception +6 to 11, Latin +10 to 19.",
      "",
      0, 0, 0, 2 },   /* 49 St.Finbar */
    { "Agility +8 to 15, Charisma +6 to 11, Streetwise +4 to 7, Speak Common and Read & Write +6 to 11. The party can cross water as if it were land.",
      "",
      0, 0, 0, 3 },   /* 50 St.Finnian */
    { "May raise the reputation by 15 to 24 in Passau, Salzburg, Linz, Steyr, Wien or Graz (nowhere else). The skills of the weapons in hand +6 to 11, Riding +6 to 11. The party can cross water as if it were land.",
      "",
      "May raise the reputation by 15 to 24 in Passau, Salzburg, Linz, Steyr, Wien, Graz, Posen or Bromberg (nowhere else). The skills of the weapons in hand +6 to 11, Riding +6 to 11. The party can cross water as if it were land.", "", 0, 3 },   /* 51 St.Florian */
    { "Heals 30% of lost Endurance and 10% of lost Strength. Perception and Endurance +6 to 11, Charisma +15 to 29, Speak Common and Woodwise +4 to 7, Healing +10 to 19, Artifice +12 to 23.",
      "Not in the game: the Virtue and the loss of cash. Instead Healing, Artifice and Woodwise.",
      "Endurance and Perception +6 to 11, Charisma +15 to 29, Speak Common +10 to 19, Virtue +25 to 49. One time in two the party loses a tenth of its money.", "", 0, 1 },   /* 52 St.Francis */
    { "Intelligence and Perception +10 to 29.",
      "",
      0, 0, 0, 14 },   /* 53 St.Gabriel */
    { "Heals 30% of lost Endurance and 20% of lost Strength. For a week no ambush takes the party by surprise (bandits and the like).",
      "",
      0, 0, 0, 22 },   /* 54 St.Genevieve */
    { "Strength +10 to 14, Agility +5 to 9, every weapon skill and Riding +25 to 49.",
      "",
      "The weapon and the armour of the one prayed for count 10 points better in quality. Strength +10 to 14, Agility +5 to 9, every weapon skill and Riding +25 to 49.", "", 1, 0 },   /* 55 St.George */
    { "Heals 30% of lost Endurance and 20% of lost Strength. The skills of the weapons in hand +15 to 24, Healing +20 to 39.",
      "",
      0, 0, 0, 0 },   /* 56 St.Gerlac */
    { "Charisma +6 to 11, Riding +5 to 9, Woodwise, Latin and Read & Write +15 to 29. At the party's next step on the map it is carried to the nearest town.",
      "",
      0, 0, 0, 4 },   /* 57 St.Gertrude o'Nivelles */
    { "Heals 40% of lost Endurance and 10% of lost Strength. Agility lowered by 4 to 7, the skills of the weapons in hand lowered by 15 to 24, Stealth +30 to 59, Woodwise +5 to 9, Streetwise +50 to 89.",
      "Woodwise and Streetwise the other way round (the text: a little Streetwise, a tremendous Woodwise); no money.",
      "Heals 40% of lost Endurance and 10% of lost Strength. Agility lowered by 4 to 7, the skills of the weapons in hand lowered by 15 to 24, Stealth +30 to 59, Woodwise +50 to 89, Streetwise +5 to 9. A party with neither florins nor groschen gets 3 to 9 groschen.", "", 0, 0 },   /* 58 St.Giles */
    { "Intelligence +5 to 9, Perception +15 to 24, Alchemy +40 to 59, Artifice +10 to 19. Costs a point of Strength (a wound) and a point of Virtue, for good.",
      "",
      0, 0, 0, 0 },   /* 59 St.Giles of Portugal */
    { "Heals 30% of lost Endurance. Latin +20 to 39, Read & Write +25 to 49.",
      "",
      0, 0, 0, 4 },   /* 60 St.Godehard */
    { "Heals 20% of lost Endurance. Speak Common +15 to 29, Artifice +10 to 19. For a week no ambush takes the party by surprise (bandits and the like).",
      "",
      0, 0, 0, 30 },   /* 61 St.Godfrey */
    { "May raise the reputation by 20 to 39 in Schleswig, Lübeck, Wismar, Rostock, Hamburg, Lüneburg or Bremen (nowhere else). Every weapon skill +15 to 29.",
      "",
      0, 0, 0, 0 },   /* 62 St.Gottschalk */
    { "Charisma +8 to 15, Alchemy and Speak Common +10 to 19, Artifice +20 to 39.",
      "",
      0, 0, 0, 13 },   /* 63 St.Gregory Thaumaturg. */
    { "May raise the reputation by 30 to 49 in Frankfurt an der Oder, Breslau, Olmütz or Teschen (nowhere else). Healing +20 to 29.",
      "",
      "May raise the reputation by 30 to 49 in Frankfurt an der Oder, Breslau, Olmütz or Teschen (nowhere else). Heals 20% of lost Endurance and Strength. Healing +20 to 29.", "", 0, 6 },   /* 64 St.Hedwig */
    { "Intelligence +8 to 23, every weapon skill +10 to 19.",
      "No reputation in its help.",
      0, 0, 0, 1 },   /* 65 St.Henry */
    { "Charisma +10 to 19, Strength +4 to 7.",
      "",
      0, 0, 0, 8 },   /* 66 St.Heribert */
    { "Perception +15 to 29.",
      "",
      0, 0, 0, 10 },   /* 67 St.Herve */
    { "Perception +20 to 59.",
      "No reputation in its help.",
      "Perception +20 to 59. By the Rhine (Basel to Xanten), may raise the reputation by 5 to 14.", "", 0, 10 },   /* 68 St.Hildegard */
    { "Polearms, Thrown and Bows +15 to 29, Missile Devices +10 to 19, Stealth +25 to 49, Woodwise +40 to 79.",
      "",
      0, 0, 0, 11 },   /* 69 St.Hubert */
    { "Charisma +15 to 29, every weapon skill and Speak Common +10 to 19, Riding +5 to 9.",
      "",
      0, 0, 0, 0 },   /* 70 St.Illtyd */
    { "Charisma +8 to 15, Streetwise +10 to 19, Speak Common +5 to 9.",
      "The reputation in Madrid and Spain: places not in the game.",
      0, 0, 0, 2 },   /* 71 St.Isidore */
    { "Heals 50% of lost Endurance and all lost Strength. Charisma +8 to 15, Healing +20 to 39.",
      "",
      0, 0, 0, 0 },   /* 72 St.Ita */
    { "Heals 20% of lost Endurance and 10% of lost Strength. Strength and Endurance +4 to 7, Charisma +3 to 5, Virtue +20 to 34.",
      "The reputation in Spain: a place not in the game; no Intelligence lost.",
      "Heals 20% of lost Endurance and 10% of lost Strength. Strength and Endurance +4 to 7, Charisma +3 to 5, Virtue +20 to 34, Intelligence lowered by 3 to 5.", "The reputation in Spain: a place not in the game.", 0, 0 },   /* 73 St.James */
    { "No help of its own: its powers work only where an encounter calls for it.",
      "",
      0, 0, 0, 3 },   /* 74 St.Januarius */
    { "A woman: heals all lost Endurance and 20% of lost Strength, and Endurance +5 to 9, Strength +4 to 7. A man: heals 30% of lost Endurance and 10% of lost Strength.",
      "",
      0, 0, 0, 0 },   /* 75 St.John of Bridlington */
    { "Charisma +8 to 15, Perception lowered by 6 to 11, Speak Common and Latin +30 to 59.",
      "",
      0, 0, 0, 0 },   /* 76 St.John Chrysostom */
    { "Alchemy, Religion, Speak Common, Latin, Read & Write, Healing, Artifice, Stealth, Streetwise and Woodwise each +1 to 4, and 1 more for every 5 points of Virtue.",
      "",
      0, 0, 0, 0 },   /* 77 St.John Climacus */
    { "@NEPOMUK Charisma +8 to 15, Intelligence +6 to 11, Speak Common +20 to 39, Streetwise +4 to 7.",
      "",
      0, 0, 0, 6 },   /* 78 St.John Nepolmuchen */
    { "Charisma +6 to 11, Artifice and Streetwise +30 to 59.",
      "",
      "Artifice and Streetwise +30 to 59; in a town, Charisma +6 to 11.", "", 0, 1 },   /* 79 St.Joseph */
    { "Endurance, Charisma, Strength, Perception, Agility and Intelligence +4 to 8, the skills of the weapons in hand +15 to 24, Alchemy, Religion, Speak Common, Latin, Read & Write, Healing, Stealth, Artifice, Woodwise and Streetwise +6 to 11.",
      "",
      0, 0, 0, 9 },   /* 80 St.Jude */
    { "Woodwise +20 to 39, Riding +6 to 11. The party can cross water as if it were land.",
      "",
      0, 0, 0, 1 },   /* 81 St.Julian, Hospitaler */
    { "Heals 40% of lost Endurance and 20% of lost Strength. Charisma +6 to 11, Speak Common +15 to 29, Riding +4 to 7, Healing +10 to 19.",
      "",
      0, 0, 0, 0 },   /* 82 St.Kessog */
    { "Strength +4 to 7, Agility +3 to 5, the skills of the weapons in hand +15 to 24, Speak Common +6 to 11, Riding +10 to 19.",
      "",
      0, 0, 0, 0 },   /* 83 St.Lasdislaus */
    { "Heals all lost Endurance. Charisma +12 to 19, Speak Common and Streetwise +10 to 19.",
      "",
      0, 0, 0, 4 },   /* 84 St.Lawrence */
    { "Heals 30% of lost Endurance and all lost Strength.",
      "",
      0, 0, 0, 1 },   /* 85 St.Lazarus */
    { "Every weapon skill +10 to 19.",
      "",
      "The weapon of the one prayed for counts 10 points better in quality. Every weapon skill +10 to 19.", "", 0, 0 },   /* 86 St.Longinius */
    { "Heals all lost Endurance and Strength. Perception +6 to 11.",
      "",
      0, 0, 0, 3 },   /* 87 St.Lucy */
    { "Heals 50% of lost Endurance and 30% of lost Strength. Healing +20 to 39.",
      "",
      0, 0, 0, 0 },   /* 88 St.Luke */
    { "Charisma +6 to 11, Perception +8 to 15, Endurance +5 to 9, Virtue +6 to 17.",
      "Endurance is raised, not spent.",
      "Costs 5 to 9 Endurance (not below 1). Charisma +6 to 11, Perception +8 to 15, Virtue +6 to 17.", "", 0, 53 },   /* 89 St.Lutgardis */
    { "A woman: heals all lost Endurance and half the lost Strength, and every attribute +3 to 6 and Alchemy, Religion, Speak Common, Latin, Read & Write, Healing, Artifice, Stealth, Streetwise and Woodwise +5 to 10. A man: heals 10% of lost Endurance and Strength.",
      "Not in the game: the protection from flames.",
      "Fire protection 90 for the one prayed for. A woman: heals all lost Endurance and half the lost Strength, and every attribute +3 to 6 and Alchemy, Religion, Speak Common, Latin, Read & Write, Healing, Artifice, Stealth, Streetwise and Woodwise +5 to 10. A man: heals 10% of lost Endurance and Strength.", "", 1, 3 },   /* 90 St.Margaret */
    { "Heals all lost Endurance and 40% of lost Strength. Healing +20 to 39, Speak Common +10 to 19.",
      "",
      0, 0, 0, 0 },   /* 91 St.Margaret of Cortona */
    { "Agility +12 to 23, Read & Write +15 to 29.",
      "",
      0, 0, 0, 0 },   /* 92 St.Mark */
    { "Heals 30% of lost Endurance and 30% of lost Strength. Charisma +8 to 15, Perception +12 to 19, Speak Common +25 to 49, Riding +20 to 39.",
      "",
      "One who wears armour has 30% of lost Endurance and Strength healed. Charisma +8 to 15, Perception +12 to 19, Speak Common +25 to 49, Riding +20 to 39.", "", 0, 10 },   /* 93 St.Martin of Tours */
    { "Intelligence +8 to 15, Latin +15 to 29, Read & Write +25 to 49. The party's standing with the Fugger and with the Medici rises by 20 to 39 each.",
      "",
      0, 0, 0, 0 },   /* 94 St.Matthew */
    { "Edged +20 to 39, Alchemy +10 to 19.",
      "",
      "An edged weapon of the one prayed for counts 10 points better in quality. Edged +20 to 39, Alchemy +10 to 19.", "", 0, 0 },   /* 95 St.Maurice */
    { "May raise the reputation in the nearest town by 20 to 39. Charisma +12 to 19, the skills of the weapons in hand +20, Healing +20 to 39.",
      "In combat it raises the skills of the weapons in hand, not the weapons and armour.",
      "Every member's weapon and armour count 10 points better in quality. May raise the reputation in the nearest town by 20 to 39. Charisma +12 to 19, the skills of the weapons in hand +20, Healing +20 to 39.", "", 1, 0 },   /* 96 St.Michael */
    { "Charisma +10 to 19, Healing +20 to 39. The party can cross water as if it were land.",
      "",
      0, 0, 0, 53 },   /* 97 St.Milburga */
    { "Every weapon skill +15 to 29, Stealth +20 to 39, Woodwise and Streetwise +10 to 19.",
      "",
      0, 0, 0, 1 },   /* 98 St.Moses the Black */
    { "May raise the reputation in the nearest town by 10 to 19. Charisma +6 to 11, Endurance +4 to 8, Flails +15 to 29.",
      "Flails and Endurance instead of Agility and Riding; the reputation anywhere, not only in his lands.",
      "May raise the reputation in Nancy (Lorraine) by 10 to 19. Agility and Charisma +6 to 11, Riding +40 to 79.", "The reputation in Russia, Greece, Sicily and Apulia: places not in the game.", 0, 2 },   /* 99 St.Nicholas */
    { "May raise the reputation by 15 to 24 in Strassburg or Basel (nowhere else). Perception +6 to 11.",
      "",
      0, 0, 0, 2 },   /* 100 St.Odilia */
    { "Perception +10 to 19, Charisma +4 to 8, every weapon skill +8 to 22, Speak Common +8 to 15.",
      "",
      0, 0, 0, 0 },   /* 101 St.Odo */
    { "Heals 30% of lost Endurance. Every weapon skill +8 to 22.",
      "",
      0, 0, 0, 0 },   /* 102 St.Olaf */
    { "@FIRE1 Heals 30% of lost Endurance and 30% of lost Strength. Charisma +6 to 11, Alchemy +10 to 19, Healing +15 to 29. The party can cross water as if it were land.",
      "Fire protection for the one prayed for (not the party), and the crossing of water, not in the text; nothing against animals.",
      "Fire protection 50 for every member. One who wears armour has 30% of lost Endurance and Strength healed. Alchemy +10 to 19, Healing +15 to 29.", "Nothing against animals.", 1, 5 },   /* 103 St.Pantaleon */
    { "Charisma +15 to 24, Strength +12 to 19, Edged, Impact, Polearms, Speak Common, Latin and Read & Write +15 to 29. @ARM2",
      "The armour is against every blow, not only missiles.",
      "Armour two levels thicker against missiles. Charisma +15 to 24, Strength +12 to 19, Edged, Impact, Polearms, Speak Common, Latin and Read & Write +15 to 29.", "", 1, 0 },   /* 104 St.Patrick */
    { "@PAUL",
      "The reputation in Greece and Malta: places not in the game.",
      0, 0, 0, 0 },   /* 105 St.Paul */
    { "@SIMPLE",
      "",
      0, 0, 0, 4 },   /* 106 St.Paul the Simple */
    { "Charisma +15 to 29.",
      "",
      0, 0, 0, 7 },   /* 107 St.Perpetua */
    { "Charisma +8 to 15, Strength +12 to 19, Perception lowered by half its maximum, Speak Common, Latin and Healing +10 to 19, Edged +15 to 29.",
      "",
      0, 0, 0, 5 },   /* 108 St.Peter */
    { "May lower the reputation in the nearest town by 1 to 4. Intelligence and Perception +6 to 11, Alchemy and Riding +10 to 19, Stealth +40 to 79.",
      "",
      0, 0, 0, 0 },   /* 109 St.Peter of Atroa */
    { "@POLYCARP",
      "",
      0, 0, 0, 3 },   /* 110 St.Polycarp */
    { "Heals all lost Endurance and Strength. Agility +8 to 15, Perception +6 to 11, Healing +50 to 139.",
      "",
      0, 0, 0, 14 },   /* 111 St.Raphael */
    { "Intelligence +15 to 29, Read & Write +30 to 89. The party can cross water as if it were land.",
      "No reputation with the Dominicans in its help.",
      0, 0, 0, 5 },   /* 112 St.Raymond Penafort */
    { "Intelligence +12 to 19, Alchemy +20 to 39, Read & Write +30 to 59. Endurance and Strength below 9 are raised to 9.",
      "",
      0, 0, 0, 0 },   /* 113 St.Raymond Lull */
    { "Heals 30% of lost Endurance. Charisma lowered by 6 to 11, Alchemy +6 to 17, Artifice +35 to 69.",
      "",
      0, 0, 0, 7 },   /* 114 St.Reinold */
    { "Heals 20% of lost Endurance and 20% of lost Strength. Healing +10 to 19, Artifice +8 to 15.",
      "",
      0, 0, 0, 2 },   /* 115 St.Roch */
    { "Heals all lost Endurance and Strength. Charisma +10 to 19.",
      "",
      0, 0, 0, 0 },   /* 116 St.Sabas the Goth */
    { "Heals 10% of lost Endurance and 10% of lost Strength. Agility +12 to 19, Bows +20 to 39, Riding +10 to 19.",
      "",
      "Heals 10% of lost Endurance and Strength. Agility +12 to 19, Bows +20 to 39, the other weapon skills +6 to 11, Riding +10 to 19.", "", 0, 2 },   /* 117 St.Sebastian */
    { "Heals 20% of lost Endurance and 10% of lost Strength. Charisma +6 to 11, Speak Common +30 to 59, Latin +20 to 39.",
      "No reputation in its help (Poland and Cracow: no such places in the game).",
      "May raise the reputation by 10 to 19 in Posen or Bromberg (nowhere else). Heals 20% of lost Endurance and 10% of lost Strength. Charisma +6 to 11, Speak Common +30 to 59, Latin +20 to 39.", "The reputation in Cracow: a place not in the game.", 0, 0 },   /* 118 St.Stanislaus */
    { "In Pressburg, may raise the reputation by 15 to 24. Charisma +6 to 11, Intelligence +8 to 15, the skills of the weapons in hand +15 to 24, Speak Common +6 to 11, Riding +10 to 19.",
      "",
      0, 0, 0, 0 },   /* 119 St.Stephen */
    { "Heals 20% of lost Endurance and 10% of lost Strength. Charisma +10 to 19, Speak Common +25 to 49.",
      "",
      0, 0, 0, 0 },   /* 120 St.Swithbert */
    { "Heals 40% of lost Endurance and 20% of lost Strength. Charisma +6 to 11.",
      "",
      0, 0, 0, 7 },   /* 121 St.Tarachus */
    { "Heals 40% of lost Endurance and 20% of lost Strength. Healing +30 to 59.",
      "",
      0, 0, 0, 0 },   /* 122 St.Thalelaeus,Merciful */
    { "Charisma +6 to 11, Endurance +4 to 8, the skills of the weapons in hand +15 to 24, Flails +15 to 29.",
      "Flails instead of Riding.",
      "Charisma +6 to 11, the skills of the weapons in hand +15 to 24, Riding +25 to 49.", "", 0, 0 },   /* 123 St.Theodore Tiro */
    { "Strength and Endurance +4 to 8, Speak Common and Woodwise +4 to 7, Healing +10 to 19, Artifice +12 to 23.",
      "",
      0, 0, 0, 0 },   /* 124 St.Thomas the Apostle */
    { "Intelligence +12 to 23, Speak Common and Latin +12 to 26, Read & Write +15 to 29, Healing +10 to 19.",
      "Not in the game: Religion and the reputation at monasteries and universities. Instead Speak Common and Healing.",
      "In a town with a monastery or a university, may raise its reputation by 10 to 19. Intelligence +12 to 23, Latin +12 to 26, Read & Write +15 to 29, Religion +25 to 49.", "", 0, 3 },   /* 125 St.Thomas Aquinas */
    { "Heals 10% of lost Endurance and 10% of lost Strength. Charisma +10 to 19, Stealth, Healing and Speak Common +6 to 11.",
      "",
      0, 0, 0, 3 },   /* 126 St.Valentine */
    { "Heals all lost Endurance and 20% of lost Strength. Charisma +8 to 15, the skills of the weapons in hand +15 to 24, Speak Common +10 to 19.",
      "",
      0, 0, 0, 0 },   /* 127 St.Victor o'Marseilles */
    { "Charisma +10 to 19, Agility +12 to 23. At the party's next step on the map it is carried to the nearest town.",
      "No Endurance lost.",
      "Charisma +10 to 19, Agility +12 to 23. One time in two, the one prayed for loses 3 to 8 Endurance (not below 1). At the party's next step on the map it is carried to the nearest town.", "", 0, 40 },   /* 128 St.Vitus */
    { "May raise the reputation by 15 to 24 in St. Joachimsthal, Bürglitz, Prag, Kuttenberg, Olmütz or Brünn (nowhere else). Perception and Strength +6 to 11, Intelligence +8 to 13, the skills of the weapons in hand +8 to 15, Riding +8 to 15.",
      "No reputation with nobles in its help.",
      0, 0, 0, 3 },   /* 129 St.Wenceslaus */
    { "Heals 20% of lost Endurance. Charisma +6 to 11, Artifice +6 to 11, Speak Common +20 to 39. For a week no ambush takes the party by surprise (bandits and the like).",
      "Not in the game: faster travel. Instead, a week safe from ambushes.",
      "The party travels twice as fast for a week. Heals 20% of lost Endurance. Charisma +6 to 11, Artifice +6 to 11, Speak Common +20 to 39.", "", 0, 2 },   /* 130 St.Wilfrid */
    { "Agility +8 to 15, Woodwise +8 to 15, Riding +10 to 19. For a week no ambush takes the party by surprise (bandits and the like).",
      "",
      0, 0, 0, 10 },   /* 131 St.Willehad */
    { "Charisma +10 to 19, Woodwise +6 to 11, Speak Common +25 to 49.",
      "",
      0, 0, 0, 0 },   /* 132 St.Willebald */
    { "Charisma +10 to 19, Woodwise +8 to 15, Speak Common +20 to 39.",
      "No reputation in its help.",
      "May raise the reputation by 10 to 19 in Groningen, Leer, Osnabrück, Zwolle, Deventer, Münster, Nymwegen or Elburg (nowhere else). Charisma +10 to 19, Woodwise +8 to 15, Speak Common +20 to 39.", "", 0, 0 },   /* 133 St.Williborord */
    { "Charisma +8 to 15, Intelligence +6 to 11, Woodwise and Riding +6 to 11, Speak Common +20 to 39.",
      "No reputation in its help.",
      0, 0, 0, 2 },   /* 134 St.Wolfgang */
    { "Heals 20% of lost Endurance and 5% of lost Strength. Perception +6 to 11, Streetwise +6 to 11.",
      "",
      0, 0, 0, 2 },   /* 135 St.Zita */
};

/* the help that depends on a fix */
static const char *special(int s, const char *tok, char *buf, size_t n)
{
    int arm = fix_is_on("armourday"), fire = fix_is_on("fireward"), pauls = fix_is_on("pauls"), nep = fix_is_on("saintrep");
    (void)s;
    if (!strcmp(tok, "FIRE1")) return arm ? "Fire protection 90 for the one prayed for." :
        fire ? "Meant as protection against fire: written where the battle reads armour, for good (it does nothing with the fire protection fix)."
             : "Meant as protection against fire: armour +32 for good, for the one prayed for (where the battle reads armour).";
    if (!strcmp(tok, "CRISPIN")) return arm ? "Every member's worn armour counts 10 points better in quality."
                                            : "Every member's worn body and leg armour +10 quality, for good, piling up with each prayer.";
    if (!strcmp(tok, "ARM1")) return arm ? "Armour one level thicker." : "Armour one level thicker, for good, piling up.";
    if (!strcmp(tok, "ARM2")) return arm ? "Armour two levels thicker." : "Armour two levels thicker, for good, piling up.";
    if (!strcmp(tok, "NEPOMUK")) return nep ? "May raise the reputation by 15 to 24 in Prag, and by 10 to 19 in St. Joachimsthal, Bürglitz, Kuttenberg, Olmütz or Brünn (nowhere else)."
        : "May raise the reputation by 10 to 19 in St. Joachimsthal, Bürglitz, Kuttenberg, Olmütz or Brünn (nowhere else); in Prag it goes to a place picked at random.";
    if (!strcmp(tok, "PAUL")) return pauls ? "Heals 10% of lost Strength. Endurance +5 to 9, Charisma +8 to 15, Speak Common +20 to 39, Latin +10 to 19, Read & Write +40 to 79." : "Heals all lost Endurance and Strength. Intelligence lowered by half its maximum.";
    if (!strcmp(tok, "SIMPLE")) return pauls ? "Heals all lost Endurance and Strength. Intelligence lowered by half its maximum." : "Intelligence +12 to 23, Speak Common and Latin +12 to 26, Read & Write +15 to 29, Healing +10 to 19.";
    if (!strcmp(tok, "POLYCARP")) return arm ? "Every member: fire protection 90." :
        fire ? "Every member: fire protection 90 for good (and armour bits the fire protection fix hides)."
             : "Every member: fire protection 90 for good, and armour +48 for good (meant as protection against fire).";
    snprintf(buf, n, "?");
    return buf;
}

/* the sainttexts fix gives this saint the help its text describes */
static int st(int s) { return INFO[s].st_does && fix_is_on("sainttexts") && (!INFO[s].st_arm || fix_is_on("armourday")); }

const char *saint_info_does(int s)
{
    static char out[600];
    if (s < 0 || s >= SAINT_INFO_N) return "";
    const char *t = st(s) ? INFO[s].st_does : INFO[s].does;
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
    return st(s) ? INFO[s].st_unlike : INFO[s].unlike;
}

int saint_info_cards(int s) { return s >= 0 && s < SAINT_INFO_N ? INFO[s].cards : 0; }

/* "" for a saint whose help holds nothing to wear off (healing, the reputation, money: St. Clare) */
const char *saint_info_note(int s)
{
    static const char *WEARS[] = { "+", "lowered", "protection", "better", "thicker" };
    const char *d = saint_info_does(s);
    for (size_t k = 0; k < sizeof WEARS / sizeof *WEARS; k++)
        if (strstr(d, WEARS[k])) return "A saint's help lasts about a day, unless said otherwise.";
    return "";
}
