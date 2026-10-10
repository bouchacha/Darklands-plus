// gamedata.h - the game's static data files, read for the new interface
// (DARKLAND.LST items, DARKLAND.ALC alchemy formulas).
#pragma once
#include <string>
#include <vector>

enum ItemKind { K_WEAPON, K_MISSILE, K_ARMOR, K_SHIELD, K_GEAR, K_TOOL, K_INSTRUMENT, K_INGREDIENT,
                K_POTION, K_RELIC, K_HORSE, K_DOCUMENT, K_QUEST, K_OTHER };

struct ItemDef {
    std::string name, shortname;
    int type = 0, flags = 0, flags2 = 0, weight = 0, quality = 0, value = 0;
    ItemKind kind = K_OTHER;
    bool mineral = false;
};

struct FormulaIngr { int count, item; };
struct Formula {
    std::string text;             // effect description
    int mystic = 0;               // mystic number
    std::vector<FormulaIngr> ingr;
};

struct GameData {
    std::vector<ItemDef> items;
    std::vector<Formula> formulas;   // 3 per potion (potion item 95 + f/3)
    bool loaded = false;
};

extern GameData gamedata;
void gamedata_load(const char *dir);
const char *kind_name(ItemKind k);
