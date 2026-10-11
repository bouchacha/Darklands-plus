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
    int flags5 = 0, rarity = 0;      // flag byte +0x24 (0x80: not sold in shops), rarity 0..12
    unsigned cats() const { return (unsigned)flags | ((unsigned)flags2 << 16); }
    ItemKind kind = K_OTHER;
    bool mineral = false;
};

struct FormulaIngr { int count, item; };
struct Formula {
    std::string text;             // effect description
    int mystic = 0;               // mystic number
    int risk = 0;                 // 0 low, 1 medium, 2 high
    std::vector<FormulaIngr> ingr;
};

struct City {
    int seed = 0;                    // +0x56: shop stock seed (with the game's party_seed)                       // DARKLAND.CTY (0x26e bytes per city; location index = city index)
    std::string name, shortname;     // UTF-8
    int size = 0, type = 0;          // type 0 free city, 1 ruled, 2 capital
    int buildings = 0;               // bit 1 town hall, 2 fortress, 4 cathedral, 5 Zeughaus, 6 slums, 7 Kloster,
                                     // 8 university, 9 pawnshop, 10 Muenz, 11 docks
    int shopq[9] = {};               // Blacksmith, Goods Merchant, Swordsmith, Armorer, Gunsmith, Bowyer,
                                     // Artificer, Jeweler, Clothmaker (0 = none)
    int x = 0, y = 0;                // map position (entry)
};

struct Saint { std::string name, shortname, text; };

struct GameData {
    std::vector<ItemDef> items;
    std::vector<Saint> saints;       // LST order (bit order of saints_known)
    std::vector<std::string> formula_names;
    std::vector<Formula> formulas;   // 3 per potion (potion item 95 + f/3)
    std::vector<City> cities;
    bool loaded = false;
};

// the game's 7-bit text ('{' = o umlaut, '|' = u umlaut ...) as UTF-8
std::string game_text(const char *s, size_t n);
extern const char *SHOP_NAMES[9];

extern GameData gamedata;
void gamedata_load(const char *dir);
const char *kind_name(ItemKind k);
