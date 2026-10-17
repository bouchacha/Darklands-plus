/* nsdraw.cpp - see nsdraw.h */
#include "nsdraw.h"
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include <set>
extern "C" {
#include "../mods/gamemodel.h"
}

std::string ns_item_line(int id)
{
    const ItemDef *d = ns_itemdef(id);
    if (!d) return "";
    if (d->type >= 0 && d->type <= 33 && (d->kind == K_WEAPON || d->kind == K_MISSILE)) {
        int w = d->type, sk = rb(G_FAR20A5, (u16)(0x75ac + w)), cp = rb(G_FAR20A5, (u16)(0x76a8 + w)), dmg = rb(G_FAR20A5, (u16)(0x76e7 + w));
        static const char *SK[7] = { "edged", "impact", "flail", "polearm", "thrown", "bow", "missile device" };
        return std::string(sk < 7 ? SK[sk] : "?") + ", " + ((cp >> 4) >= 2 ? "two hands" : "one hand") + ", damage " + std::to_string(dmg) + ", penetration " + std::to_string(cp & 15);
    }
    if (d->type >= 67 && d->type <= 84) {
        static const char *HN[6] = { "none", "soft", "stiff", "scale", "mail", "plate" };
        int h = rb(G_FAR20A5, (u16)(0x781e + d->type));
        return std::string(d->type <= 75 ? "Covers body and arms" : "Covers the legs") + ", hardness " + std::to_string(h) + " (" + (h >= 0 && h < 6 ? HN[h] : "?") + ")";
    }
    return "";
}

/* what a thing is good for, where that is not plain from its kind (ingredients: the potions made
 * with it; the general goods: what in the game asks for them). Read from the game's code: of the
 * general goods only rope and lockpicks are asked for by cards (count_items, s029 0e76:0c76: the
 * party's, so one is enough), and an instrument by St. Cecilia (the type flag 0x20000000, s039
 * 165c:1024); nothing asks for a torch, candle, lantern, iron spike, grappling hook or clock. */
std::string ns_item_use(int id)
{
    const ItemDef *d = ns_itemdef(id);
    if (!d) return "";
    if (d->kind == K_INGREDIENT) {
        std::vector<std::string> in, known;
        std::set<int> seen;
        for (int f = 0; f < (int)gamedata.formulas.size(); f++) {
            bool uses = false;
            for (const FormulaIngr &g : gamedata.formulas[f].ingr) if (g.item == id) uses = true;
            if (!uses) continue;
            int potion = 95 + f / 3;
            bool k = false;
            for (int m = 0; m < gm_party_size() && m < 5; m++) if (gm_member_present(m) && gm_knows_formula(m, f)) k = true;
            if (seen.insert(potion).second) in.push_back(ns_item_name(potion));
            if (k) { std::string n = ns_item_name(potion); bool dup = false; for (auto &x : known) if (x == n) dup = true; if (!dup) known.push_back(n); }
        }
        if (in.empty()) return "No formula uses it.";
        std::string r = "Goes into ";
        for (size_t i = 0; i < in.size(); i++) r += (i ? (i + 1 == in.size() ? " and " : ", ") : "") + in[i];
        r += ".";
        if (!known.empty()) {
            r += " The party knows how to make ";
            for (size_t i = 0; i < known.size(); i++) r += (i ? (i + 1 == known.size() ? " and " : ", ") : "") + known[i];
            r += ".";
        }
        return r;
    }
    switch (id) {
    case 0x3b: return "For climbing: over a town's walls at night, into a castle and the like, the best climber goes up first and lets it down for the others (without one, each climbs alone). One in the party is enough.";
    case 0x40: return "For picking locks: shops at night, doors, chests, a cell. One in the party is enough.";
    case 0x41: case 0x42: return "St. Cecilia, the patron of music, raises the party's standing in a town greatly for a member who carries an instrument.";
    case 0x39: case 0x3a: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
        return "Nothing in the game asks for one (no card, no saint): it is worth only what it sells for.";
    }
    return "";
}
