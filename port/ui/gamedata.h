// gamedata.h - the game's static data files, read for the new interface
// (DARKLAND.LST items, DARKLAND.ALC alchemy formulas).
#pragma once
#include <string>
#include <vector>

enum ItemKind { K_WEAPON, K_MISSILE, K_ARMOR, K_SHIELD, K_GEAR, K_TOOL, K_INSTRUMENT, K_INGREDIENT,
                K_POTION, K_RELIC, K_HORSE, K_DOCUMENT, K_QUEST, K_OTHER };

struct ItemDef {
    std::string name, shortname;     // name: as shown (gamedata_names: in full, armour by where it is worn); shortname as in the game
    std::string game_name;           // as in DARKLAND.LST and the game's own screens and texts (match the game's text with this)
    std::string plain;               // as shown, without where an armour is worn ("Chainmail")
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
    int size = 0, type = 0;          // type 0..2 (free, ruled, capital in one community reading - unconfirmed: Hamburg and Luebeck are 2, Kassel and Dresden 0)
    int buildings = 0;               // bit 1 town hall, 2 fortress, 4 cathedral, 5 Zeughaus, 6 slums, 7 Kloster,
                                     // 8 university, 9 pawnshop, 10 Muenz, 11 docks
    int shopq[9] = {};               // Blacksmith, Goods Merchant, Swordsmith, Armorer, Gunsmith, Bowyer,
                                     // Artificer, Jeweler, Clothmaker (0 = none)
    int x = 0, y = 0;                // map position (entry)
};

struct Saint { std::string name, shortname, text, game_name; };   // name as shown ("St. Gregory Thaumaturgus"), game_name as the game writes it

struct GameData {
    std::vector<ItemDef> items;
    std::vector<Saint> saints;       // LST order (bit order of saints_known)
    std::vector<std::string> formula_names;        // as shown (in full)
    std::vector<std::string> formula_game_names;   // as the game writes them (match the game's text with these)
    std::vector<Formula> formulas;   // 3 per potion (potion item 95 + f/3)
    std::vector<City> cities;
    bool loaded = false;
};

// the game's 7-bit text ('{' = o umlaut, '|' = u umlaut ...) as UTF-8
std::string game_text(const char *s, size_t n);
extern const char *SHOP_NAMES[9];
/* the quality a shop's weapons, armour and most other wares come out at: (the thing's base, 25 for
   most, + the shop's rating) / 2 (s032 1462:0efa item_quality; ammunition and ingredients are 25) */
static inline int shop_wares_q(int rating) { return (25 + rating) / 2; }
/* the shops a card can open (none opens the gunsmith, the jeweller or the clothmaker: the bowyer
   sells the guns; the clothmakers' street offers only "leave") */
static inline bool shop_open_ever(int k) { return k != 4 && k != 7 && k != 8; }

extern GameData gamedata;
void gamedata_load(const char *dir);
/* the names as shown, from the game's: in full where the game cut them short for its 20 letters
   ("Studded Lthr", "Illum. Gospel", "St.Gregory Thaumaturg.", "Raimundus Ls'"), and armour by where
   it is worn ("Chainmail (body)" for "V:Chainmail"); settings display.full_names and
   display.armour_worn. Run again when either changes. */
void gamedata_names(void);
/* a text of the game's (a card's) with the names cut short written out as gamedata_names shows them */
std::string gamedata_spell_text(const std::string &t);
/* a place's name as shown: a city's in full (DARKLAND.CTY), else the location's own (+0x26, a few
   of them cut short by the game's 11 letters: written out); "" for none */
std::string loc_display_name(int loc);
const char *kind_name(ItemKind k);          /* a kind's name, for a list of them ("Weapons") */
const char *kind_name_one(ItemKind k);      /* for one thing ("Weapon") */
// where a saint's description turns from the legend to what the saint does (an offset into text)
size_t saint_effect_at(const std::string &text);
void ui_saint_info(int s);                      // the description, what the help really does, where they differ (ImGui, wrapped)
const char *saint_encounters_line(int s);        // whether encounters let a member call on saint s
