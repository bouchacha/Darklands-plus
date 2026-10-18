/* nsdraw.cpp - see nsdraw.h */
#include "nsdraw.h"
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include <set>
extern "C" {
#include "../mods/gamemodel.h"
}

std::string ns_quality_words(int id, int qual)
{
    const ItemDef *d = ns_itemdef(id);
    if (d && d->kind == K_POTION) return "";                  /* (the dots on its flask tell its grade) */
    return ", quality " + std::to_string(qual);
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

/* a weapon or an armour against what member m has in that place now (the equipment slots of the
 * member record: weapon +0x3d/+0x44, missile +0x0e/+0x46, body +0x37/+0x3b, legs +0x38/+0x3c,
 * shield +0x48/+0x47: type and quality); "" for other things */
std::string ns_item_compare(int id, int q, int m)
{
    const ItemDef *d = ns_itemdef(id);
    if (!d || m < 0 || m >= 5 || !gm_member_present(m)) return "";
    char nm[24]; gm_member_name(m, nm, sizeof nm);
    std::string who = ns_gtext(nm);
    u16 b = G_MEMBER(m);
    auto slot = [&](int toff, int qoff, int &t, int &wq) { t = (s8)g_rb((u16)(b + toff)); wq = g_rb((u16)(b + qoff)); return t >= 0; };
    auto name_of = [&](int t, int wq) {                     /* the record in use of that type */
        GmItem it;
        for (int i = 0; i < gm_item_count(m); i++) if (gm_item(m, i, &it) && it.type == t && it.qual == wq) return ns_item_name(it.id);
        return std::string("?");
    };
    auto cmpq = [&](int a, int c) { return a > c ? "better made" : a < c ? "less well made" : "as well made"; };
    static const char *HN[6] = { "none", "soft", "stiff", "scale", "mail", "plate" };
    if (d->type >= 67 && d->type <= 84) {
        bool vital = d->type <= 75;
        int t, wq, h = rb(G_FAR20A5, (u16)(0x781e + d->type));
        if (!slot(vital ? 0x37 : 0x38, vital ? 0x3b : 0x3c, t, wq) || t < 67 || t > 84) return who + " wears nothing there.";
        int wh = rb(G_FAR20A5, (u16)(0x781e + t));
        std::string n = name_of(t, wq);
        if (n.size() > 2 && n[1] == ':') n = n.substr(2);
        int s1 = (h > wh) - (h < wh), s2 = (q > wq) - (q < wq);
        return who + " wears " + n + " (" + (wh >= 0 && wh < 6 ? HN[wh] : "?") + ", q" + std::to_string(wq) + "): this is " +
               (s1 > 0 ? "harder" : s1 < 0 ? "softer" : "as hard") + (s1 == s2 ? " and " : " but ") + cmpq(q, wq) + ".";
    }
    if (d->kind == K_SHIELD) {
        int t, wq;
        if (!slot(0x48, 0x47, t, wq)) return who + " carries no shield.";
        return who + " carries " + name_of(t, wq) + " (q" + std::to_string(wq) + "): this is " + cmpq(q, wq) + ".";
    }
    if (d->type >= 0 && d->type <= 62 && (d->kind == K_WEAPON || d->kind == K_MISSILE)) {
        static const char *SKN[7] = { "Edged", "Impact", "Flail", "Polearm", "Thrown", "Bow", "Missile Device" };
        int w = d->type, sk = rb(G_FAR20A5, (u16)(0x75ac + w)), smin = rb(G_FAR20A5, (u16)(0x7765 + w));
        bool missile = sk == 5 || sk == 6;
        std::string r = who + "'s " + (sk < 7 ? SKN[sk] : "?") + " skill: " + std::to_string(sk < 7 ? gm_member_skill(m, sk) : 0) + ".";
        int str, strmax; gm_member_attr(m, 1, &str, &strmax);
        if (smin > 0 && strmax < smin) r += " Too heavy: it needs strength " + std::to_string(smin) + ".";
        int t, wq;
        if (slot(missile ? 0x0e : 0x3d, missile ? 0x46 : 0x44, t, wq) && t <= 62) {
            int cp = rb(G_FAR20A5, (u16)(0x76a8 + t)), dmg = rb(G_FAR20A5, (u16)(0x76e7 + t));
            r += " In hand now: " + name_of(t, wq) + " (damage " + std::to_string(dmg) + ", penetration " + std::to_string(cp & 15) + ", q" + std::to_string(wq) + ").";
        } else r += missile ? " No bow or crossbow ready now." : " Nothing in hand now.";
        return r;
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
