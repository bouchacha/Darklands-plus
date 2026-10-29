// gamedata.cpp - see gamedata.h
#include "gamedata.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <strings.h>
#ifdef _WIN32
#define strcasecmp _stricmp
#endif

GameData gamedata;
static std::vector<std::string> formula_raw;      /* the formulae's names as DARKLAND.LST writes them (the game's own letters) */
extern "C" int gamedata_formula_raw(int f, char *out, int n)   /* (for text the game shows: a card's $Text2) */
{
    if (f < 0 || f >= (int)formula_raw.size() || n < 1) return 0;
    snprintf(out, (size_t)n, "%s", formula_raw[(size_t)f].c_str());
    return 1;
}
extern "C" const char *datamods_redirect(const char *rel, const char *game_path);
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
}

static FILE *open_ci(const char *dir, const char *name)
{
    { const char *m = datamods_redirect(name, ""); if (m) { FILE *f = fopen(m, "rb"); if (f) return f; } }   /* a mod's file */
    DIR *d = opendir(dir);
    char path[1200];
    if (d) {
        struct dirent *e;
        while ((e = readdir(d))) {
            if (!strcasecmp(e->d_name, name)) {
                snprintf(path, sizeof path, "%s/%s", dir, e->d_name);
                closedir(d);
                return fopen(path, "rb");
            }
        }
        closedir(d);
    }
    snprintf(path, sizeof path, "%s/%s", dir, name);
    return fopen(path, "rb");
}

static std::vector<unsigned char> read_all(FILE *f)
{
    std::vector<unsigned char> v;
    if (!f) return v;
    unsigned char b[4096];
    size_t n;
    while ((n = fread(b, 1, sizeof b, f)) > 0) v.insert(v.end(), b, b + n);
    fclose(f);
    return v;
}

static int w16(const unsigned char *p) { return p[0] | (p[1] << 8); }

const char *SHOP_NAMES[9] = { "Blacksmith", "Goods merchant", "Swordsmith", "Armorer", "Gunsmith", "Bowyer",
                              "Artificer", "Jeweller", "Clothmaker" };

std::string game_text(const char *s, size_t n)
{
    std::string o;
    for (size_t i = 0; i < n && s[i]; i++) {
        switch (s[i]) {
        case 0x1f: o += "\xc3\xa4"; break;    // a umlaut (the game's own code for it)
        case '{': o += "\xc3\xb6"; break;     // o umlaut
        case '|': o += "\xc3\xbc"; break;     // u umlaut
        case '}': o += "\xc3\xa4"; break;     // a umlaut
        case '~': o += "\xc3\x9f"; break;     // sharp s
        default: if ((unsigned char)s[i] >= 32 && (unsigned char)s[i] < 127) o += s[i]; else if (s[i] == '\n' || s[i] == '\r') o += ' ';
        }
    }
    return o;
}

static ItemKind classify(const ItemDef &d)
{
    int f = d.flags;
    if (f & 0x4000) return K_DOCUMENT;
    if (f & 0x2000) return K_HORSE;
    if (f & 0x1000) return K_RELIC;
    if (f & 0x800) return K_POTION;
    if (f & 0x400) return K_INGREDIENT;
    if ((f & 0x300) == 0x300) return K_INSTRUMENT;
    if (f & 0x200) return K_TOOL;
    if (f & 0x100) return K_GEAR;
    /* the item type numbers (LST +30) group the equipment; the flags leave crossbows, guns,
       ammunition and the soft armours unmarked */
    int t = d.type;
    if (t >= 95 && t <= 97) return K_SHIELD;
    if (t >= 67 && t <= 84) return K_ARMOR;
    if ((t >= 23 && t <= 33) || (t >= 64 && t <= 66)) return K_MISSILE;
    if (t >= 0 && t <= 22 && (f & 0x0f)) return K_WEAPON;
    if (f & 0x80) return K_SHIELD;
    if (f & 0x40) return K_ARMOR;
    if (f & 0x30) return K_MISSILE;
    if (f & 0x0f) return K_WEAPON;
    if (d.flags2 & 0x80) return K_QUEST;
    return K_OTHER;
}

const char *kind_name(ItemKind k)
{
    static const char *n[] = { "Weapons", "Missile weapons", "Armor", "Shields", "Gear", "Tools", "Instruments",
                               "Alchemical ingredients", "Potions", "Relics", "Horses", "Documents", "Curiosities", "Other" };
    return n[k];
}
const char *kind_name_one(ItemKind k)
{
    static const char *n[] = { "Weapon", "Missile weapon", "Armor", "Shield", "Gear", "Tool", "Instrument",
                               "Alchemical ingredient", "Potion", "Relic", "Horse", "Document", "Curiosity", "Item" };
    return n[k];
}

void gamedata_load(const char *dir)
{
    std::vector<unsigned char> lst = read_all(open_ci(dir, "DARKLAND.LST"));
    if (lst.size() > 3) {
        int n = lst[0];
        for (int i = 0; i < n && 3 + (i + 1) * 46 <= (int)lst.size(); i++) {
            const unsigned char *r = &lst[3 + i * 46];
            ItemDef d;
            d.game_name = d.name = d.plain = std::string((const char *)r, strnlen((const char *)r, 20));
            d.shortname = std::string((const char *)r + 20, strnlen((const char *)r + 20, 10));
            d.type = w16(r + 30); d.flags = w16(r + 32); d.flags2 = w16(r + 34);
            d.weight = r[37]; d.quality = r[38]; d.value = w16(r + 44);
            d.flags5 = r[36]; d.rarity = r[39];
            d.mineral = (d.flags2 & 0x1000) != 0;
            d.kind = classify(d);
            gamedata.items.push_back(d);
        }
        /* then saint full names, short names, formula full names, short names */
        size_t pos = 3 + (size_t)n * 46;
        int ns = lst[1], nf = lst[2];
        auto cstr = [&](void) { std::string r; while (pos < lst.size() && lst[pos]) r += (char)lst[pos++]; pos++; return r; };
        for (int i = 0; i < ns && pos < lst.size(); i++) { Saint t; t.game_name = t.name = game_text(cstr().c_str(), 64); gamedata.saints.push_back(t); }
        for (int i = 0; i < ns && pos < lst.size(); i++) gamedata.saints[i].shortname = game_text(cstr().c_str(), 64);
        formula_raw.clear();
        for (int i = 0; i < nf && pos < lst.size(); i++) { std::string r = cstr(); formula_raw.push_back(r); gamedata.formula_game_names.push_back(game_text(r.c_str(), 64)); }
        gamedata.formula_names = gamedata.formula_game_names;
    }
    std::vector<unsigned char> snt = read_all(open_ci(dir, "DARKLAND.SNT"));
    if (snt.size() > 1)
        for (int i = 0; i < snt[0] && i < (int)gamedata.saints.size() && 1 + (size_t)(i + 1) * 360 <= snt.size(); i++)
            gamedata.saints[i].text = game_text((const char *)&snt[1 + (size_t)i * 360], 360);
    std::vector<unsigned char> alc = read_all(open_ci(dir, "DARKLAND.ALC"));
    if (alc.size() > 1) {
        int n = alc[0];
        for (int i = 0; i < n && 1 + (i + 1) * 0x68 <= (int)alc.size(); i++) {
            const unsigned char *r = &alc[1 + i * 0x68];
            Formula f;
            f.text = std::string((const char *)r, strnlen((const char *)r, 0x50));
            f.mystic = w16(r + 0x50);
            f.risk = w16(r + 0x52);
            for (int k = 0; k < 5; k++) {
                int c = r[0x54 + 4 * k], it = w16(r + 0x56 + 4 * k);
                if (c > 0) f.ingr.push_back({ c, it });
            }
            gamedata.formulas.push_back(f);
        }
    }
    std::vector<unsigned char> cty = read_all(open_ci(dir, "DARKLAND.CTY"));
    if (cty.size() > 1) {
        int n = cty[0];
        for (int i = 0; i < n && 1 + (i + 1) * 0x26e <= (int)cty.size(); i++) {
            const unsigned char *r = &cty[1 + i * 0x26e];
            City c;
            c.shortname = game_text((const char *)r, 0x20);
            c.name = game_text((const char *)r + 0x20, 0x20);
            c.size = w16(r + 0x40); c.type = w16(r + 0x58); c.buildings = w16(r + 0x5e);
            for (int k = 0; k < 9; k++) c.shopq[k] = r[0x62 + k];
            c.x = w16(r + 0x42); c.y = w16(r + 0x44);
            c.seed = w16(r + 0x56);
            for (int k = 0; k < 5; k++) c.docks[k] = (short)w16(r + 0x4a + 2 * k);
            gamedata.cities.push_back(c);
        }
    }
    /* every place's name from the file (DARKLAND.LOC: a count, then 0x3a-byte records, the name at +0x26):
       the copy in the game's memory is not there in every screen (a battle in a mine or castle: the
       names read from it were rubbish, and a save made there was called so) */
    std::vector<unsigned char> loc = read_all(open_ci(dir, "DARKLAND.LOC"));
    if (loc.size() > 2) {
        int n = loc[0] | loc[1] << 8;
        for (int i = 0; i < n && 2 + (i + 1) * 0x3a <= (int)loc.size(); i++) {
            const unsigned char *r = &loc[2 + i * 0x3a];
            char b[21]; memcpy(b, r + 0x26, 20); b[20] = 0;
            gamedata.loc_names.push_back(b);
        }
    }
    gamedata.loaded = !gamedata.items.empty();
    gamedata_names();
}

/* ---------------------------------------------------------------- the names in full
   Only what is certain is written out: the alchemists whose names the game gives in full in
   another formula (Petrus Bonus, Nicolas Flammel, Rob de Ketene, Michael Scot, John of
   Rupescissa) or who are the famous ones of that name (Albertus Magnus, Raimundus Lullus, Arnald
   of Villanova, Gerard of Cremona); "Rufinus of G", "Alfred of S", "Simon C", "Vincent Bs",
   "Roger of H", "Leonard of M" and "Nicolaus of A" stay as the game has them. */
static const char *const ITEM_FULL[][2] = {
    { "Studded Lthr", "Studded Leather" }, { "Cuirboulli", "Cuirbouilli" },           /* (armour: after V: / L:) */
    { "East. Black Bean", "Eastern Black Bean" }, { "Phleqmatic Base", "Phlegmatic Base" }, { "Melanc. Base", "Melancholic Base" },
    { "Essence o'Grace", "Essence of Grace" }, { "Birth Confess.", "Birth Confession" }, { "Illum. Gospel", "Illuminated Gospel" },
    { "S.Mary's Tears", "St. Mary's Tears" }, { "S.Arnulf GreatSwrd", "St. Arnulf's Greatsword" }, { "S.Catherine Pain", "St. Catherine's Pain" },
    { "S.Olaf Battleaxe", "St. Olaf's Battleaxe" }, { "S.George GreatSwrd", "St. George's Greatsword" }, { "S.Dunstan Hammer", "St. Dunstan's Hammer" },
    { "Spar of S.Erasmus", "Spar of St. Erasmus" }, { "Staff of S.Patrick", "Staff of St. Patrick" }, { "S.Hubert's Bow", "St. Hubert's Bow" },
    { "S.Edward's Ring", "St. Edward's Ring" }, { "S.Emygdius Finger", "St. Emygdius' Finger" }, { "S.Gabriel's Horn", "St. Gabriel's Horn" },
    { "S.Ita's Needle", "St. Ita's Needle" }, { "S.Kessog Medallion", "St. Kessog's Medallion" }, { "S.Odo's Testament", "St. Odo's Testament" },
    { "S.Odilia's Oil", "St. Odilia's Oil" }, { "S.Raphael's Water", "St. Raphael's Water" }, { "S.Swithbert's Foot", "St. Swithbert's Foot" },
    { "S.Willehad's Shoe", "St. Willehad's Shoe" }, { "S.Thealeaus' Spoon", "St. Thealeaus' Spoon" },
};
static const char *const SAINT_FULL[][2] = {
    { "St.Christina, Astonish.", "St. Christina the Astonishing" }, { "St.Gertrude o'Nivelles", "St. Gertrude of Nivelles" },
    { "St.Gregory Thaumaturg.", "St. Gregory Thaumaturgus" }, { "St.Thalelaeus,Merciful", "St. Thalelaeus the Merciful" },
    { "St.Victor o'Marseilles", "St. Victor of Marseilles" }, { "St.Edward, Confessor", "St. Edward the Confessor" },
    { "St.Julian, Hospitaler", "St. Julian the Hospitaller" },
};
static const char *const FORMULA_PARTS[][2] = {     /* (pieces of a formula's name) */
    { "Gerard C's", "Gerard of Cremona's" }, { "Gerard of C's", "Gerard of Cremona's" }, { "Petrus Bs'", "Petrus Bonus'" },
    { "Nicolas F's", "Nicolas Flammel's" }, { "Smargdn.", "Smaragdine" }, { "Raimundus Ls'", "Raimundus Lullus'" },
    { "Raimundus L's", "Raimundus Lullus'" }, { "Arnald of V's", "Arnald of Villanova's" }, { "Albertus Ms'", "Albertus Magnus'" },
    { "Albertus M's", "Albertus Magnus'" }, { "Rob. de K's", "Rob de Ketene's" }, { "John of R's", "John of Rupescissa's" },
    { "John Rupesc.'s", "John of Rupescissa's" }, { "Transform.", "Transformation" }, { "Michael S'", "Michael Scot's" },
    { "Essence o'Grace", "Essence of Grace" },
};
extern "C" int config_get_int(const char *key, int def);

static std::string saint_spaced(const std::string &n)        /* "St.Jude" -> "St. Jude" */
{
    if (n.size() > 3 && n.compare(0, 3, "St.") == 0 && n[3] != ' ') return "St. " + n.substr(3);
    return n;
}

void gamedata_names(void)
{
    bool full = config_get_int("display.full_names", 1) != 0, worn = config_get_int("display.armour_worn", 1) != 0;
    for (ItemDef &d : gamedata.items) {
        std::string n = d.game_name, slot;
        bool armour = d.kind == K_ARMOR && n.size() > 2 && (n[0] == 'V' || n[0] == 'L') && n[1] == ':';
        if (armour) { slot = n.substr(0, 2); n = n.substr(2); }
        if (full) for (const auto &m : ITEM_FULL) if (n == m[0]) { n = m[1]; break; }
        d.plain = n;
        if (!armour) d.name = n;
        else if (full && worn) d.name = n + (slot[0] == 'V' ? " (body)" : " (legs)");
        else d.name = slot + n;
    }
    for (Saint &t : gamedata.saints) {
        t.name = t.game_name;
        if (!full) continue;
        for (const auto &m : SAINT_FULL) if (t.game_name == m[0]) { t.name = m[1]; break; }
        t.name = saint_spaced(t.name);
    }
    gamedata.formula_names = gamedata.formula_game_names;
    if (full)
        for (std::string &f : gamedata.formula_names)
            for (const auto &m : FORMULA_PARTS) { size_t at = f.find(m[0]); if (at != std::string::npos) f.replace(at, strlen(m[0]), m[1]); }
}

std::string gamedata_spell_text(const std::string &t)
{
    if (!config_get_int("display.full_names", 1)) return t;
    std::string r = t;
    auto swap = [&](const char *from, const char *to) {
        size_t at = 0, fl = strlen(from), tl = strlen(to);
        while ((at = r.find(from, at)) != std::string::npos) { r.replace(at, fl, to); at += tl; }
    };
    for (const auto &m : ITEM_FULL) swap(m[0], m[1]);
    for (const auto &m : SAINT_FULL) swap(m[0], m[1]);
    for (const auto &m : FORMULA_PARTS) swap(m[0], m[1]);
    for (size_t i = 0; i + 3 < r.size(); i++)                  /* "St.Jude" -> "St. Jude" */
        if (r.compare(i, 3, "St.") == 0 && r[i + 3] >= 'A' && r[i + 3] <= 'Z' && (i == 0 || !isalpha((unsigned char)r[i - 1]))) r.insert(i + 3, " ");
    return r;
}

std::string loc_display_name(int loc)
{
    if (loc >= 0 && loc < (int)gamedata.cities.size() && !gamedata.cities[loc].name.empty()) return gamedata.cities[loc].name;
    char b[24] = "";
    if (loc >= 0 && loc < (int)gamedata.loc_names.size()) snprintf(b, sizeof b, "%s", gamedata.loc_names[loc].c_str());
    else { u32 r = loc >= 0 ? g_location(loc) : 0; if (!r) return ""; g_str((u16)(r >> 16), (u16)((u16)r + 0x26), b, sizeof b); }
    if (!b[0]) return "";
    static const struct { const char *cut, *full; } CUT[] = { { "Recklinghsn", "Recklinghausen" }, { "Marienwerd", "Marienwerder" } };
    for (const auto &c : CUT) if (!strcmp(b, c.cut)) return c.full;
    return game_text(b, sizeof b);
}

/* (for C: a place's name as shown, in the game's letters turned to plain ones: "Gorlitz"; "" for none) */
extern "C" void gamedata_loc_name(int loc, char *out, int n)
{
    std::string t = loc_display_name(loc);
    int k = 0;
    for (size_t i = 0; i < t.size() && k < n - 1; i++) {
        unsigned char c = (unsigned char)t[i];
        if (c < 0x80) out[k++] = (char)c;
        else if (c == 0xc3 && i + 1 < t.size()) {          /* (a letter with an umlaut or accent, as UTF-8: its plain letter) */
            static const char *PL = "AAAAAAACEEEEIIIIDNOOOOOxOUUUUYTsaaaaaaaceeeeiiiidnooooo/ouuuuyty";
            unsigned char d = (unsigned char)t[++i];
            if (d >= 0x80 && d < 0xc0) out[k++] = PL[d - 0x80];
        }
    }
    out[k] = 0;
}

/* a town's number for its cards: loc_query 0x21, the city's +0x56 + party_seed (16-bit, as the game) */
int town_seed(int loc)
{
    if (loc < 0 || loc >= (int)gamedata.cities.size()) return 0;
    return (u16)(gamedata.cities[loc].seed + g_rw(G_PARTY_SEED));
}
/* the quality an alchemist's ($Alche00, s066 190c:04c9: (seed + 4) mod 25 + 1, unsigned) or a university's
   ($Unive00, s056 18c0:0137: ((seed + 7) & 15) + 14) better Philosopher's Stone has in town loc: the same
   all game; either card sells it only when it is better than the party's, and says so only after its roll */
extern "C" int town_stone_offer(int loc, int university)
{
    if (loc < 0 || loc >= (int)gamedata.cities.size()) return 0;
    unsigned s = (unsigned)town_seed(loc);
    return university ? (int)(((s + 7) & 15) + 14) : (int)((u16)(s + 4) % 25 + 1);
}
