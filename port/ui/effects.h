// effects.h - what is at work on the party now (ui/saints.cpp makes the list from the game's
// events): a saint's boost of an attribute or a skill, or the lowering some saints bring with it,
// a weapon potion, and a companion's time with the party. Shown in the margin's "Blessings and potions"
// (ui/margins.cpp) and on the character sheet (ui/sheetview.cpp: the raised values, how long).
#pragma once
#include <string>
#include <vector>

struct Eff {
    int m;              /* the member */
    std::string what;   /* "+10 Perception", "Deadly Blade: +4 damage", "stays with the party" */
    long left;          /* hours left */
    int kind;           /* the event's kind: 0x48 attribute, 0x49 skill, 0x4a weapon potion, 0x43 a companion's stay */
    int k = -1;         /* 0x48 / 0x49: which attribute or skill */
    int amount = 0;     /* 0x48 / 0x49: by how much (a lowering: negative) */
    int icon = -1;      /* the rest: its picture (EI_*, ui_effect_icon), -1 none */
    int src = 0;        /* where it came from, as the game records it: 1 a potion, 2 a saint; 0 not recorded (an attribute's or skill's boost) */
    std::string val;    /* the rest: what the picture's value says, short ("90", "+5q", "x2") */
};
enum { EI_FIRE, EI_ARMOR, EI_ARMOR_Q, EI_MISSILE_ARMOR, EI_WEAPON_Q, EI_DAMAGE, EI_PENETRATION, EI_TRAVEL, EI_TRAVEL_WOODS, EI_NO_DEATH, EI_STAYS };
std::vector<Eff> effects_now(int size);   /* (members 0 .. size-1) */
