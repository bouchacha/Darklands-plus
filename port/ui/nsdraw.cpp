/* nsdraw.cpp - see nsdraw.h */
#include "nsdraw.h"
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include <set>
#include <vector>
#include <algorithm>
extern "C" {
#include "../mods/gamemodel.h"
}

std::string ns_quality_words(int id, int qual)
{
    const ItemDef *d = ns_itemdef(id);
    if (!d || !ns_shows_quality(id, qual)) return "";        /* (as the lists: a potion's dots tell its grade; gear's quality only its price) */
    return ", " + std::to_string(qual) + "q";
}

std::string ns_item_line(int id, int q)
{
    const ItemDef *d = ns_itemdef(id);
    if (!d) return "";
    if (d->type >= 95 && d->type <= 97) {                  /* a shield: its defence, by size and quality (mods/gamemodel.h) */
        if (q < 0) q = d->quality;
        return "Defence " + std::to_string(gm_shield_defence(d->type, q, 0)) + " against blows, " + std::to_string(gm_shield_defence(d->type, q, 1)) +
               " against shots and throws; unusable with a two-handed weapon";
    }
    if (d->type >= 0 && d->type <= 33 && (d->kind == K_WEAPON || d->kind == K_MISSILE)) {
        int w = d->type, sk = rb(G_FAR20A5, (u16)(0x75ac + w)), cp = rb(G_FAR20A5, (u16)(0x76a8 + w)), dmg = rb(G_FAR20A5, (u16)(0x76e7 + w));
        static const char *SK[7] = { "Edged", "Impact", "Flail", "Polearm", "Thrown", "Bow", "Missile Device" };
        int tm = rb(G_FAR20A5, (u16)(0x7669 + w)), rl = rb(G_FAR20A5, (u16)(0x77e3 + w));
        std::string r = std::string(sk < 7 ? SK[sk] : "?") + ", " + ((cp >> 4) >= 2 ? "two hands" : "one hand") + ", damage " + std::to_string(dmg) + ", penetration " + std::to_string(cp & 15);
        if (tm) r += ", time for a blow " + std::to_string(tm);
        if (rl) r += std::string(", time to ") + (w >= 30 ? "reload " : w >= 27 ? "draw and loose " : "throw ") + std::to_string(rl);
        return r;
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
        for (int i = 0; i < gm_item_count(m); i++)
            if (gm_item(m, i, &it) && it.type == t && it.qual == wq) { const ItemDef *w = ns_itemdef(it.id); return w ? ns_gtext(w->plain.c_str()) : ns_item_name(it.id); }   /* (armour: "Scale") */
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
        int s1 = (h > wh) - (h < wh), s2 = (q > wq) - (q < wq);
        return who + " wears " + n + " (" + (wh >= 0 && wh < 6 ? HN[wh] : "?") + ", " + std::to_string(wq) + "q): this is " +
               (s1 > 0 ? "harder" : s1 < 0 ? "softer" : "as hard") + (s1 == s2 ? " and " : " but ") + cmpq(q, wq) + ".";
    }
    if (d->kind == K_SHIELD) {
        int t, wq;
        if (!slot(0x48, 0x47, t, wq)) return who + " carries no shield.";
        return who + " carries " + name_of(t, wq) + " (" + std::to_string(wq) + "q): this is " + cmpq(q, wq) + ".";
    }
    if (d->type >= 0 && d->type <= 62 && (d->kind == K_WEAPON || d->kind == K_MISSILE)) {
        static const char *SKN[7] = { "Edged", "Impact", "Flail", "Polearm", "Thrown", "Bow", "Missile Device" };
        int w = d->type, sk = rb(G_FAR20A5, (u16)(0x75ac + w)), smin = rb(G_FAR20A5, (u16)(0x7765 + w));
        bool missile = sk == 5 || sk == 6;
        std::string r = who + "'s " + (sk < 7 ? SKN[sk] : "?") + " skill: " + std::to_string(sk < 7 ? gm_member_skill(m, sk) : 0) + ".";
        int str, strmax; gm_member_attr(m, 1, &str, &strmax);
        if (smin > 0 && strmax < smin) r += " Too heavy: it needs Strength " + std::to_string(smin) + ".";
        int t, wq;
        if (slot(missile ? 0x0e : 0x3d, missile ? 0x46 : 0x44, t, wq) && t <= 62) {
            int cp = rb(G_FAR20A5, (u16)(0x76a8 + t)), dmg = rb(G_FAR20A5, (u16)(0x76e7 + t));
            r += " In hand now: " + name_of(t, wq) + " (damage " + std::to_string(dmg) + ", penetration " + std::to_string(cp & 15) + ", " + std::to_string(wq) + "q).";
        } else r += missile ? " No missile weapon ready now." : " Nothing in hand now.";
        return r;
    }
    return "";
}

/* a weapon or an armour against the whole party (the shared pack: whom it would suit) */
std::string ns_item_compare_party(int id, int q)
{
    const ItemDef *d = ns_itemdef(id);
    if (!d) return "";
    std::vector<int> ms;
    for (int m = 0; m < gm_party_size() && m < 5; m++) if (gm_member_present(m) && gm_member_status(m) != 3) ms.push_back(m);
    auto nm = [](int m) { char b[24]; gm_member_name(m, b, sizeof b); return ns_gtext(b); };
    auto join = [](const std::vector<std::string> &v) {
        std::string r;
        for (size_t i = 0; i < v.size(); i++) r += (i == 0 ? "" : i + 1 == v.size() ? " and " : ", ") + v[i];
        return r;
    };
    if (d->type >= 67 && d->type <= 84) {
        bool vital = d->type <= 75;
        int h = rb(G_FAR20A5, (u16)(0x781e + d->type));
        std::vector<std::string> better, none;
        for (int m : ms) {
            u16 b = G_MEMBER(m);
            int t = (s8)g_rb((u16)(b + (vital ? 0x37 : 0x38))), wq = g_rb((u16)(b + (vital ? 0x3b : 0x3c)));
            if (t < 67 || t > 84) { none.push_back(nm(m)); continue; }
            int wh = rb(G_FAR20A5, (u16)(0x781e + t));
            if (h > wh || (h == wh && q > wq)) better.push_back(nm(m));
        }
        std::string r;
        if (!better.empty()) r = "Harder or better made than what " + join(better) + (better.size() == 1 ? " wears" : " wear") + " there.";
        if (!none.empty()) r += (r.empty() ? "" : " ") + join(none) + (none.size() == 1 ? " wears" : " wear") + " nothing there.";
        return r.empty() ? "Everyone wears as good or better there." : r;
    }
    if (d->kind == K_SHIELD) {
        std::vector<std::string> better, none;
        for (int m : ms) {
            u16 b = G_MEMBER(m);
            int t = (s8)g_rb((u16)(b + 0x48)), wq = g_rb((u16)(b + 0x47));
            if (t < 0) none.push_back(nm(m)); else if (q > wq) better.push_back(nm(m));
        }
        std::string r;
        if (!better.empty()) r = "Better made than the shield of " + join(better) + ".";
        if (!none.empty()) r += (r.empty() ? "" : " ") + join(none) + (none.size() == 1 ? " carries" : " carry") + " no shield.";
        return r.empty() ? "Everyone's shield is as good or better." : r;
    }
    if (d->type >= 0 && d->type <= 62 && (d->kind == K_WEAPON || d->kind == K_MISSILE)) {
        static const char *SKN[7] = { "Edged", "Impact", "Flail", "Polearm", "Thrown", "Bow", "Missile Device" };
        int w = d->type, sk = rb(G_FAR20A5, (u16)(0x75ac + w)), smin = rb(G_FAR20A5, (u16)(0x7765 + w));
        if (sk >= 7) return "";
        std::vector<std::pair<int, std::string>> v; std::vector<std::string> heavy;
        for (int m : ms) {
            v.push_back({ gm_member_skill(m, sk), nm(m) });
            int str, strmax; gm_member_attr(m, 1, &str, &strmax);
            if (smin > 0 && strmax < smin) heavy.push_back(nm(m));
        }
        std::sort(v.begin(), v.end(), [](const std::pair<int, std::string> &a, const std::pair<int, std::string> &b) { return a.first > b.first; });
        std::string r = std::string(SKN[sk]) + " skill: ";
        for (size_t i = 0; i < v.size(); i++) r += (i ? ", " : "") + v[i].second + " " + std::to_string(v[i].first);
        r += ".";
        if (!heavy.empty()) r += " Too heavy for " + join(heavy) + " (it needs Strength " + std::to_string(smin) + ").";
        return r;
    }
    return "";
}

extern "C" int fix_is_on(const char *id);
#include "../mods/fixes.h"
std::string ns_potion_effect(int id);
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
        /* each potion with the grades of its formulae (1 to 3, the dots on its flask) that take it */
        struct Use { int potion; bool need[3]; bool known[3]; };
        std::vector<Use> uses;
        for (int f = 0; f < (int)gamedata.formulas.size(); f++) {
            bool takes = false;
            for (const FormulaIngr &g : gamedata.formulas[f].ingr) if (g.item == id) takes = true;
            if (!takes) continue;
            int potion = 95 + f / 3; size_t k = 0;
            while (k < uses.size() && uses[k].potion != potion) k++;
            if (k == uses.size()) uses.push_back({ potion, { false, false, false }, { false, false, false } });
            uses[k].need[f % 3] = true;
            for (int m = 0; m < gm_party_size() && m < 5; m++) if (gm_member_present(m) && gm_knows_formula(m, f)) uses[k].known[f % 3] = true;
        }
        if (uses.empty()) return "No formula uses it.";
        auto grades = [](const bool g[3], const char *all) {   /* "" (or `all`) for every grade; else " (grade 2 only)" */
            int n = 0; for (int i = 0; i < 3; i++) n += g[i];
            if (n == 3) return std::string(all);
            std::string r = n == 1 ? " (grade " : " (grades ";
            bool first = true; for (int i = 0; i < 3; i++) if (g[i]) { r += (first ? "" : " and ") + std::to_string(i + 1); first = false; }
            return r + " only)";
        };
        std::string r = "Goes into ", kn;
        for (size_t i = 0; i < uses.size(); i++) {
            r += (i ? (i + 1 == uses.size() ? " and " : ", ") : "") + ns_item_name(uses[i].potion) + grades(uses[i].need, "");
            if (uses[i].known[0] || uses[i].known[1] || uses[i].known[2]) {
                std::string g = grades(uses[i].known, " (every grade)");
                if (g.size() > 6 && g.compare(g.size() - 6, 6, " only)") == 0) g = g.substr(0, g.size() - 6) + ")";
                kn += (kn.empty() ? "" : ", ") + ns_item_name(uses[i].potion) + g;
            }
        }
        r += ".";
        if (!kn.empty()) r += " The party knows the formulae for " + kn + ".";
        return r;
    }
    if (id >= 95 && id <= 116) return ns_potion_effect(id);
    switch (id) {
    case 0x3b: return "For climbing: over a town's walls at night, into a castle and the like, the best climber goes up first and lets it down for the others (without one, each climbs alone). One in the party is enough.";
    case 0x40: return "For picking locks: shops at night, doors, chests, a cell. One in the party is enough.";
    case 0x41: case 0x42: return "St. Cecilia, the patron of music, raises the party's standing in a town greatly for a member who carries an instrument.";
    case 0x39: case 0x3a: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
        return "Nothing in the game asks for one (no card, no saint): it is worth only what it sells for.";
    /* the quest things and trophies (read from the cards' scripts: add_item, count_items, take_item) */
    case 0x8c: case 0x8d: case 0x8e: case 0x8f: case 0x90: case 0x91:
        return "One of the Six Keys: the gate to the last chamber of the Baphomet's lair opens only for a party that carries all six. Keep it.";
    case 0x92: return "A trophy: the boars' tusks, thought to be good-luck charms. Nothing in the game asks for them and no shop buys them.";
    case 0x93: return "A trophy. Nothing in the game asks for it.";
    case 0x94: return "A skeleton near a cemetery wants its bone back: give it this and it will talk with you.";
    case 0x95: return "The dragon's gold cup, taken back from the thief: brandish it before the dragon and it will not breathe fire on its treasure.";
    }
    if (id >= 129 && id <= 139) return "Wanted by whoever sent you to fetch it: bring it to them (the journal, F12, says who and where).";
    return "";
}

/* what a potion does, as the game's code does it (use_potion s031 1462:43b2; thrown: effect_spell
 * s019; hit_damage s014 18bf), the grades 1/2/3 being the qualities 25/35/45 of a brew. The weapon
 * oils are the member's, not a weapon's: the game keeps them in one byte of the member (+0x34:
 * damage and penetration) that every hit the member lands uses, whatever the weapon (bows and
 * fists too), whatever the potion's label says. */
std::string ns_potion_effect(int id)
{
    bool fixed = fix_is_on("potions") != 0;
    std::string day = fixed ? " for about a day (19 to 21 hours)." : " permanently, adding up with the next dose (the original game never removes them).";
    switch (id) {
    case 95: return "Thrown in a battle: a reeking cloud (up to 4 by 4 paces). Whoever it hits loses half their Agility, Perception and weapon skills and everyone else in the cloud a quarter, allies and enemies alike, while it lasts (longer at higher grades).";
    case 96: return "Thrown: blinding powder. The target loses a third of their Agility, Perception and weapon skills and is blinded (cannot shoot, throw or take orders well); those beside them lose a quarter. Lasts longer at higher grades.";
    case 97: return "Thrown: a cloud of black smoke. Shots through any cloud hit far less often (a fifth of the chance), and fighting from inside one costs 15 to hit. Lasts longer at higher grades. (The battle menu never offers it to the party: it is used on cards, and enemies throw it.)";
    case 98: return "Thrown: itching powder. Weapon skills fall by five times the armour worn on the body and limbs: the heavier the armour, the worse the itch. Lasts longer at higher grades.";
    case 99: return "Thrown: acid that eats the target's armour, lowering its quality by 1d8 + 4, 6 or 8 (grade 1, 2, 3); armour eaten to nothing is destroyed, and where there is no armour (or damage is left over) it wounds the target. Also dissolves locks.";
    case 100: return "Thrown: a poison cloud (up to 2 by 2 paces) whose wounds no armour stops, worse for weaker targets; stronger at higher grades.";
    case 101: return "Thrown: a blinding flash. The target and every enemy of the thrower on that floor who faces it are blinded for a while (longer at higher grades); the thrower's allies are spared.";
    case 102: return "Thrown: it bursts with great force: damage 1d8 + 4, 6 or 8 (grade 1, 2, 3), penetration 6, 7 or 8 on the target and less around it; it also wears the armour down by a quarter of the damage.";
    case 103: return "Thrown: burning pitch that no armour stops: damage 1d6 + 4, 6 or 8 (grade 1, 2, 3), a quarter of that further off, minus a tenth of the armour's quality (and of Firewall's protection); it also wears the armour down.";
    case 104: return "Thrown: a pool of sticky sludge (up to 4 by 4 paces) that slows whoever crosses it to 1/2, 3/8 or 1/4 of their pace (grade 1, 2, 3), for a long while.";
    case 105: if (fix_is_on("oilweapons")) return std::string("Used (P in the inventory) by a member: +3, +4 or +5 damage (grade 1, 2, 3) on every hit they land with any weapon except an impact weapon or a flail, as the manual says. Damage only, not the chance to hit;") + day;
              return "Used (P in the inventory) by a member: +3, +4 or +5 damage (grade 1, 2, 3) on every hit they land, with any weapon: the game ties it to the member, not a blade, so bows, guns, impact weapons and fists gain too (its label says non-impact weapons). Damage only, not the chance to hit;" + day;
    case 106: if (fix_is_on("oilweapons")) return "Used by a member: +3, +4 or +5 penetration (grade 1, 2, 3) on every hit they land with a blade or a polearm, as the manual says, " + std::string(fixed ? "up to 15 in all" : "wrapping past 15 in the original") + ";" + day;
              return "Used by a member: +3, +4 or +5 penetration (grade 1, 2, 3) on every hit they land, with any weapon (its label says blades and poles), " + std::string(fixed ? "up to 15 in all" : "wrapping past 15 in the original (a bow's 6 + 10 hit as 0)") + ";" + day;
    case 107: if (fix_is_on("greatpower")) return std::string("Used by a member: the weapon in hand counts as 14, 22 or 30 points higher in quality (grade 1, 2, 3; up to 99), ") + (fix_is_on("oilweapons") ? "if it is an impact weapon, a flail or a handgun, as the manual says" : "including bows, guns and fists") + ", for about a day (19 to 21 hours). Each 10 points of quality add one step of damage. A second dose adds to the first.";
              return "Used by a member, on every hit they land, with any weapon: grade 1 +14 penetration; grade 2 +1 damage and +6 penetration; grade 3 +1 damage and +14 penetration (the game adds 14, 22 or 30 to the byte holding both). " + std::string(fixed ? "Penetration counts up to 15 in all" : "In the original, penetration wraps past 15, so grades 1 and 3 lowered a bow's") + ";" + day;
    case 108: return "Used by a member: +14, +22 or +30 (grade 1, 2, 3) to Bows, Thrown Weapons and Missile Devices (up to 99), for about a day (19 to 21 hours). A second dose adds to the first.";
    case 109: if (fix_is_on("armourday")) return "Used by a member: their armour counts 20, 25 or 30 points better in quality and 1, 2 or 2 levels thicker (grade 1, 2, 3) against every blow and shot, as the manual says, for about a day (19 to 21 hours). A second dose while one is active keeps the stronger.";
              return "Used by a member: +1, +2 or +3 (grade 1, 2, 3) to their armour against every hit, blow or shot, permanently: the game never removes it, and doses add up.";
    case 110: return "Drinking it yields just one florin: it is meant for cards, where an encounter asks for it (to purify, or to make gold).";
    case 111: case 112: case 113: case 114: {
        static const char *const ATT[4] = { "Perception", "Endurance", "Strength", "Agility" };
        static const char *const AMT[4] = { "9-14, 15-21 or 22-29", "6-11, 8-13 or 10-15", "6-11, 8-13 or 10-15", "6-11, 8-15 or 10-17" };
        int a = id - 111;
        std::string t = std::string("Drunk: ") + ATT[a] + " up by " + AMT[a] + " (grade 1, 2, 3) for about a day: the current value rises, past the maximum if need be; the maximum itself does not change. ";
        if (a == 1 || a == 2) t += std::string("On a wounded member it heals: ") + (fix_is_on("blessing") ? "when it wears off, only the part still above the maximum is lost, so the healing stays." : "but when it wears off the game takes the whole amount back, healed or not (the Blessings fix keeps the healed part).");
        else t += "When it wears off, what it added is lost.";
        return t + " A second dose while the first is active adds only what goes beyond it.";
    }
    case 115: return "Drunk: permanently heals 50, 70 or 90% of the Strength lost to wounds (grade 1, 2, 3), and raises Endurance by 9-14, 15-21 or 22-29 for about 30 hours.";
    case 116: {
        if (fix_is_on("armourday")) return "Drunk: protection against fire for about a day (19 to 21 hours): Arabian Fire does 5, 7 or 9 points less (grade 1, 2, 3). A second dose keeps the stronger.";
        return std::string("Drunk: permanent protection against fire (the game never removes it): Arabian Fire does 5, 7 or 9 points less (grade 1, 2, 3).") + (fix_is_on("fireward") ? "" : " In the original it also adds 16 to the armour permanently (a bug Darklands+ fixes)."); }
    }
    return "";
}

void ns_item_strip(NsDraw &P, float y0, const NsStripThing &t)
{
    const ItemDef *d = ns_itemdef(t.id);
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT;
    P.icon(71, y0 + 8, 11.0f, t.id, t.qual);
    float rw = t.right.empty() ? 0 : P.width(P.fo, NsDraw::FS_BODY, t.right) + 6;
    P.text(P.f, NsDraw::FS_HEAD, 79, y0 + 1.5f, ink, ns_item_name(t.id), -1, 314 - rw);
    if (!t.right.empty()) P.text(P.fo, NsDraw::FS_BODY, 0, y0 + 1.8f, t.right_col, t.right, 314);
    /* (the weight only of what can be worn or wielded: the load is the equipped things' alone,
       equip_weight s031 18e7:140a; what is merely carried weighs nothing) */
    bool wearable = d && (d->kind == K_WEAPON || d->kind == K_MISSILE || d->kind == K_ARMOR || d->kind == K_SHIELD);
    std::string l1 = std::string(d ? kind_name_one(d->kind) : "") + ns_quality_words(t.id, t.qual) + (wearable ? ", weight " + std::to_string(t.weight) + " when equipped" : std::string());
    if (t.count > 1) l1 += " each, " + std::to_string(t.count) + " of them";
    if (!t.status.empty()) l1 += "; " + t.status;
    float y = y0 + 10;
    P.text(P.fo, NsDraw::FS_BODY, 79, y, soft, l1); y += 7;
    std::string l2 = ns_item_line(t.id, t.qual);
    if (!l2.empty()) { P.text(P.fo, NsDraw::FS_BODY, 79, y, soft, l2); y += 7; }
    if (d && d->kind == K_INGREDIENT) {                     /* an ingredient: the potions it goes into, as pictures */
        /* each potion has three formulae, of grade 1, 2 and 3 (the dots on its flask); under each
           picture three pips, one a grade: filled where that grade's formula takes this ingredient,
           gold where someone in the party knows that formula */
        struct Use { int potion; int need[3]; bool known[3]; };
        std::vector<Use> uses;
        for (int f = 0; f < (int)gamedata.formulas.size(); f++) {
            int need = 0;
            for (const FormulaIngr &g : gamedata.formulas[f].ingr) if (g.item == t.id) need += g.count;
            if (!need) continue;
            int potion = 95 + f / 3; size_t k = 0;
            while (k < uses.size() && uses[k].potion != potion) k++;
            if (k == uses.size()) uses.push_back({ potion, { 0, 0, 0 }, { false, false, false } });
            uses[k].need[f % 3] = need;
            for (int m = 0; m < gm_party_size() && m < 5; m++) if (gm_member_present(m) && gm_knows_formula(m, f)) uses[k].known[f % 3] = true;
        }
        if (uses.empty()) { P.text(P.fo, NsDraw::FS_BODY, 79, y, soft, "No formula uses it."); return; }
        const char *lab = "Goes into";
        P.text(P.fo, NsDraw::FS_SMALL, 79, y + 3.0f, soft, lab);
        float x = 79 + P.width(P.fo, NsDraw::FS_SMALL, lab) + 3, sz = 9.0f;
        float step = fminf(11.0f, (314 - x) / (float)uses.size());
        int hov = -1;
        for (size_t k = 0; k < uses.size(); k++) {
            const Use &u = uses[k];
            float cx = x + k * step + step * 0.5f, cy = y + sz * 0.5f;
            bool any_known = u.known[0] || u.known[1] || u.known[2];
            P.icon(cx, cy, sz, u.potion);
            if (!any_known) P.rect(cx - sz * 0.55f, cy - sz * 0.62f, cx + sz * 0.55f, cy + sz * 0.62f, IM_COL32(230, 214, 178, 185));   /* (no formula with it known: washed out) */
            for (int g = 0; g < 3; g++) {                     /* the grades' pips */
                ImVec2 pc = NsDraw::w(cx + (g - 1) * 2.4f, cy + sz * 0.62f + 1.6f);
                float r = 0.85f * P.k;
                if (u.need[g]) P.dl->AddCircleFilled(pc, r, u.known[g] ? IM_COL32(196, 146, 30, 255) : IM_COL32(70, 48, 26, 230));
                else P.dl->AddCircle(pc, r, IM_COL32(120, 96, 66, 150), 0, 0.25f * P.k);
            }
            if (P.over(cx - step * 0.5f, cy - sz * 0.6f, cx + step * 0.5f, cy + sz * 0.6f + 3.5f)) hov = (int)k;
        }
        std::string line;
        if (hov >= 0) {                                         /* the potion's grades that take it, how many a batch, which are known */
            const Use &u = uses[hov];
            line = ns_item_name(u.potion) + ": ";
            bool first = true;
            for (int g = 0; g < 3; g++) if (u.need[g]) {
                char b[64]; snprintf(b, sizeof b, "%sgrade %d (%d a batch%s)", first ? "" : ", ", g + 1, u.need[g], u.known[g] ? ", known" : "");
                line += b; first = false;
            }
            int other = 0; for (int g = 0; g < 3; g++) if (!u.need[g]) other++;
            if (other) line += other == 2 ? "; the other two grades don't use it" : "; the other grade doesn't use it";
        } else {
            int nk = 0; for (const Use &u : uses) nk += u.known[0] || u.known[1] || u.known[2];
            line = "Pips: the grades (1 to 3) whose formula uses it, gold where the party knows it. Point at a potion for more.";
            if (!nk) line = "Nobody in the party knows a formula with it. Pips: the grades (1 to 3) that use it; point at a potion for more.";
        }
        P.text(P.fo, NsDraw::FS_SMALL, 79, y + sz + 4.2f, hov >= 0 ? ink : NsDraw::INK_DIM, line, -1, 314);
        return;
    }
    std::string use = ns_item_use(t.id);                    /* what it is good for (rope, lockpicks ...) */
    if (!use.empty()) P.para(P.fo, NsDraw::FS_BODY, 79, y, 235, soft, use);
    else if (t.cmp_who >= 0 || t.cmp_who == -2) {          /* a weapon or an armour: against what the member (-2: the party) has now */
        std::string cmp = t.cmp_who == -2 ? ns_item_compare_party(t.id, t.qual) : ns_item_compare(t.id, t.qual, t.cmp_who);
        if (!cmp.empty()) P.para(P.fo, NsDraw::FS_BODY, 79, y, 235, ink, cmp);
    }
}
