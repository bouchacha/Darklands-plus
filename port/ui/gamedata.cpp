// gamedata.cpp - see gamedata.h
#include "gamedata.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <strings.h>
#ifdef _WIN32
#define strcasecmp _stricmp
#endif

GameData gamedata;

static FILE *open_ci(const char *dir, const char *name)
{
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

const char *SHOP_NAMES[9] = { "Blacksmith", "Goods merchant", "Swordsmith", "Armourer", "Gunsmith", "Bowyer",
                              "Artificer", "Jeweller", "Clothmaker" };

std::string game_text(const char *s, size_t n)
{
    std::string o;
    for (size_t i = 0; i < n && s[i]; i++) {
        switch (s[i]) {
        case '{': o += "\xc3\xb6"; break;     // o umlaut
        case '|': o += "\xc3\xbc"; break;     // u umlaut
        case '}': o += "\xc3\xa4"; break;     // a umlaut
        case '~': o += "\xc3\x9f"; break;     // sharp s
        default: if ((unsigned char)s[i] >= 32 && (unsigned char)s[i] < 127) o += s[i];
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
    if (f & 0x80) return K_SHIELD;
    if (f & 0x40) return K_ARMOR;
    if (f & 0x30) return K_MISSILE;
    if (f & 0x0f) return K_WEAPON;
    if (d.flags2 & 0x80) return K_QUEST;
    return K_OTHER;
}

const char *kind_name(ItemKind k)
{
    static const char *n[] = { "Weapons", "Missile weapons", "Armour", "Shields", "Gear", "Tools", "Instruments",
                               "Alchemical ingredients", "Potions", "Relics", "Horses", "Documents", "Curiosities", "Other" };
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
            d.name = std::string((const char *)r, strnlen((const char *)r, 20));
            d.shortname = std::string((const char *)r + 20, strnlen((const char *)r + 20, 10));
            d.type = w16(r + 30); d.flags = w16(r + 32); d.flags2 = w16(r + 34);
            d.weight = r[37]; d.quality = r[38]; d.value = w16(r + 44);
            d.mineral = (d.flags2 & 0x1000) != 0;
            d.kind = classify(d);
            gamedata.items.push_back(d);
        }
    }
    std::vector<unsigned char> alc = read_all(open_ci(dir, "DARKLAND.ALC"));
    if (alc.size() > 1) {
        int n = alc[0];
        for (int i = 0; i < n && 1 + (i + 1) * 0x68 <= (int)alc.size(); i++) {
            const unsigned char *r = &alc[1 + i * 0x68];
            Formula f;
            f.text = std::string((const char *)r, strnlen((const char *)r, 0x50));
            f.mystic = w16(r + 0x50);
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
            gamedata.cities.push_back(c);
        }
    }
    gamedata.loaded = !gamedata.items.empty();
}
