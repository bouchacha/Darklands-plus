// party.cpp - a short word on what each attribute and skill is for (the character sheet's Skills
// page shows it under the pointer, with the whole party's values).
static const char *ATTR_HELP[7] = {
    "Health and stamina. Wounds take it away; at 0 the character falls unconscious. Rest brings it back.",
    "Bodily strength. Weapons and armor need a minimum Strength (shown in the shops' tooltips), and Endurance and "
    "Strength together set how much a character can carry. Bad wounds take Strength too; at 0 the character dies.",
    "Speed and deftness, of the body and of the hands.",
    "Alertness: noticing things, people and dangers.",
    "Wits and learning.",
    "Presence: how people take to the character.",
    "The saints' goodwill. Praying to a saint uses it up; it comes back with time.",
};
static const char *SKILL_HELP[19] = {
    0, 0, 0, 0, 0, 0, 0,
    "Mixing potions from the formulae a character knows: a better alchemist fails less often (the character sheet's Formulae tab shows each member's chance).",
    "Knowledge of the faith and its saints: learning of saints, dealings with the Church.",
    "Moral purity, valued by the saints and the Church. Sinful deeds lower it.",
    "Talking with ordinary people in their own tongue: asking, persuading, bluffing.",
    "The tongue of the Church and of scholars: priests, monks, universities.",
    "Literacy: letters, books and formulae.",
    "Tending wounds and sickness.",
    "Mechanical skill: locks, traps and devices.",
    "Moving and hiding unseen and unheard.",
    "The ways of the city streets and their underworld.",
    "Handling horses.",
    "Life in the wild: finding the way, hunting, reading tracks.",
};

const char *ui_attr_help(int a) { return a >= 0 && a < 7 ? ATTR_HELP[a] : ""; }       /* (the character sheet's strip) */
const char *ui_skill_help(int k) { return k >= 0 && k < 19 && SKILL_HELP[k] ? SKILL_HELP[k] : ""; }
