/* nsdraw.cpp - see nsdraw.h */
#include "../runtime/config.h"
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

extern "C" const char *gm_skill_short(int k);
std::string ns_item_line(int id, int q)
{
    const ItemDef *d = ns_itemdef(id);
    if (!d) return "";
    if (d->type >= 95 && d->type <= 97) {                  /* a shield: its defence, by size and quality (mods/gamemodel.h) */
        if (q < 0) q = d->quality;
        return "Defense " + std::to_string(gm_shield_defence(d->type, q, 0)) + " against blows, " + std::to_string(gm_shield_defence(d->type, q, 1)) +
               " against shots and throws; unusable with a two-handed weapon";
    }
    if (item_weaponlike(d) && d->type <= 33) {
        int w = d->type, sk = rb(G_FAR20A5, (u16)(0x75ac + w)), cp = rb(G_FAR20A5, (u16)(0x76a8 + w)), dmg = rb(G_FAR20A5, (u16)(0x76e7 + w));
        int tm = rb(G_FAR20A5, (u16)(0x7669 + w)), rl = rb(G_FAR20A5, (u16)(0x77e3 + w));
        std::string r = (sk < 7 ? "{" + std::string(gm_skill_short(sk)) + "}" : std::string("?")) + ", " + ((cp >> 4) >= 2 ? "two hands" : "one hand") + ", damage " + std::to_string(dmg) + ", penetration " + std::to_string(cp & 15);
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
    std::string who = rich_member(m);                       /* (the member's head: ui/richtext.h) */
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
    if (item_weaponlike(d)) {
        int w = d->type, sk = rb(G_FAR20A5, (u16)(0x75ac + w)), smin = rb(G_FAR20A5, (u16)(0x7765 + w));
        bool missile = sk == 5 || sk == 6;
        std::string r = who + " " + (sk < 7 ? "{" + std::string(gm_skill_short(sk)) + ":" + std::to_string(gm_member_skill(m, sk)) + "}" : std::string("skill: ?")) + ".";
        int str, strmax; gm_member_attr(m, 1, &str, &strmax);
        if (smin > 0 && strmax < smin) r += " Too heavy: it needs {Strength:" + std::to_string(smin) + "}.";
        int t, wq;
        if (slot(missile ? 0x0e : 0x3d, missile ? 0x46 : 0x44, t, wq) && t <= 62) {
            int cp = rb(G_FAR20A5, (u16)(0x76a8 + t)), dmg = rb(G_FAR20A5, (u16)(0x76e7 + t));
            r += " In hand now: " + name_of(t, wq) + " (damage " + std::to_string(dmg) + ", penetration " + std::to_string(cp & 15) + ", " + std::to_string(wq) + "q).";
        } else r += missile ? " No missile weapon equipped." : " Nothing in hand now.";
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
    auto nm = [](int m) { return rich_member(m); };          /* (each member's head: ui/richtext.h) */
    auto join = [](const std::vector<std::string> &v) {
        std::string r;
        for (size_t i = 0; i < v.size(); i++) r += (i == 0 ? "" : " ") + v[i];
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
    if (item_weaponlike(d)) {
        int w = d->type, sk = rb(G_FAR20A5, (u16)(0x75ac + w)), smin = rb(G_FAR20A5, (u16)(0x7765 + w));
        if (sk >= 7) return "";
        std::vector<std::pair<int, int>> v; std::vector<std::string> heavy;
        for (int m : ms) {
            v.push_back({ gm_member_skill(m, sk), m });
            int str, strmax; gm_member_attr(m, 1, &str, &strmax);
            if (smin > 0 && strmax < smin) heavy.push_back(nm(m));
        }
        std::stable_sort(v.begin(), v.end(), [](const std::pair<int, int> &a, const std::pair<int, int> &b) { return a.first > b.first; });
        std::string r = "{" + std::string(gm_skill_short(sk)) + "}: ";
        for (size_t i = 0; i < v.size(); i++) r += (i ? "  " : "") + rich_member(v[i].second, std::to_string(v[i].first));
        r += ".";
        if (!heavy.empty()) r += " Too heavy for " + join(heavy) + " (it needs {Strength:" + std::to_string(smin) + "}).";
        return r;
    }
    return "";
}

extern "C" int fix_is_on(const char *id);
#include "../mods/fixes.h"
std::string ns_potion_effect(int id);
/* how much the encounters ask for a thing (a potion: the card scripts that list it among an
   option's potions, card_items, counted from the game's code) or a saint (mods/saintinfo.h):
   the line, and its colour - many green, some blue, a few brown, none grey */
ImU32 ns_tier_colour(int n) { return n >= 20 ? IM_COL32(34, 110, 30, 255) : n >= 5 ? IM_COL32(36, 78, 150, 255) : n >= 1 ? IM_COL32(150, 92, 30, 255) : IM_COL32(140, 124, 104, 255); }
std::string ns_item_encounters(int id, ImU32 *col)
{
    if (id < 95 || id > 116) return "";
    static const unsigned char CARDS[22] = { 61, 42, 54, 5, 15, 2, 73, 41, 11, 7, 0, 1, 0, 0, 1, 6, 1, 1, 0, 0, 1, 6 };
    int n = CARDS[id - 95];
    if (col) *col = ns_tier_colour(n);
    return n >= 20 ? "Many encounters let you use it." : n >= 5 ? "Some encounters let you use it." : n >= 1 ? "A few encounters let you use it." : "No encounter asks for it.";
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
    if (d->kind == K_INGREDIENT) return "";                 /* (the potions it goes into are pictures: ns_ingredient_row) */
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
    if (id >= 129 && id <= 139) return "Wanted by whoever sent you to fetch it: bring it to them (the quests on the world map, M, say who and where).";
    return "";
}

/* an ingredient: the potions it goes into, as pictures (each potion has three formulae, of grade 1,
   2 and 3, the dots on its flask; under each picture three pips, one a grade: filled where that
   grade's formula takes this ingredient, gold where someone in the party knows that formula), and
   under them a line about the one under the pointer; from x0 to x1 at y, the height it takes */
float ns_ingredient_row(NsDraw &P, float x0, float y, float x1, int id)
{
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT;
    struct Use { int potion; int need[3]; bool known[3]; };
    std::vector<Use> uses;
    for (int f = 0; f < (int)gamedata.formulas.size(); f++) {
        int need = 0;
        for (const FormulaIngr &g : gamedata.formulas[f].ingr) if (g.item == id) need += g.count;
        if (!need) continue;
        int potion = 95 + f / 3; size_t k = 0;
        while (k < uses.size() && uses[k].potion != potion) k++;
        if (k == uses.size()) uses.push_back({ potion, { 0, 0, 0 }, { false, false, false } });
        uses[k].need[f % 3] = need;
        for (int m = 0; m < gm_party_size() && m < 5; m++) if (gm_member_present(m) && gm_knows_formula(m, f)) uses[k].known[f % 3] = true;
    }
    if (uses.empty()) { P.text(P.fo, NsDraw::FS_BODY, x0, y, soft, "No formula uses it."); return 7; }
    const char *lab = "Goes into";
    P.text(P.fo, NsDraw::FS_SMALL, x0, y + 3.0f, soft, lab);
    float x = x0 + P.width(P.fo, NsDraw::FS_SMALL, lab) + 3, sz = 9.0f;
    float step = fminf(11.0f, (x1 - x) / (float)uses.size());
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
        line = "Dots: the grades (1 to 3) whose formula uses it, gold where the party knows it. Point at a potion for more.";
        if (!nk) line = "Nobody in the party knows a formula with it. Dots: the grades (1 to 3) that use it; point at a potion for more.";
    }
    return sz + 4.2f + P.para(P.fo, NsDraw::FS_SMALL, x0, y + sz + 4.2f, x1 - x0, hov >= 0 ? ink : NsDraw::INK_DIM, line);
}

/* what a potion does, as the game's code does it (use_potion s031 1462:43b2; thrown: effect_spell
 * s019; hit_damage s014 18bf), the grades 1/2/3 being the qualities 25/35/45 of a brew. The weapon
 * oils are the member's, not a weapon's: the game keeps them in one byte of the member (+0x34:
 * damage and penetration) that every hit the member lands uses, whatever the weapon (bows and
 * fists too), whatever the potion's label says. */
std::string ns_potion_effect(int id)
{
    bool fixed = fix_is_on("potions") != 0;
    std::string day = fixed ? " for about a day (19 to 21 hours)." : " permanently, adding up with the next dose.";
    switch (id) {
    case 95: return "Thrown in a battle: a reeking cloud (up to 4 by 4 paces). Whoever it hits loses half their {Agility}, {Perception} and weapon skills and everyone else in the cloud a quarter, allies and enemies alike, while it lasts (longer at higher grades).";
    case 96: return "Thrown: blinding powder. The target loses a third of their {Agility}, {Perception} and weapon skills and is blinded (cannot shoot, throw or take orders well); those beside them lose a quarter. Lasts longer at higher grades.";
    case 97: return "Thrown: a cloud of black smoke. Shots through any cloud hit far less often (a fifth of the chance), and fighting from inside one costs 15 to hit. Lasts longer at higher grades. (The battle menu never offers it to the party: it is used on cards, and enemies throw it.)";
    case 98: return "Thrown: itching powder. Weapon skills fall by five times the armor worn on the body and limbs: the heavier the armor, the worse the itch. Lasts longer at higher grades.";
    case 99: return "Thrown: acid that eats the target's armor, lowering its quality by 4, 6 or 8 (grade 1, 2, 3) plus up to 8 more at random; armor eaten to nothing is destroyed, and where there is no armor (or damage is left over) it wounds the target. Also dissolves locks.";
    case 100: return "Thrown: a poison cloud (up to 2 by 2 paces) whose wounds no armor stops, worse for weaker targets; stronger at higher grades.";
    case 101: return "Thrown: a blinding flash. The target and every enemy of the thrower on that floor who faces it are blinded for a while (longer at higher grades); the thrower's allies are spared.";
    case 102: return "Thrown: it bursts with great force: damage 4, 6 or 8 (grade 1, 2, 3) plus up to 8 more at random, penetration 6, 7 or 8 on the target and less around it; it also wears the armor down by a quarter of the damage.";
    case 103: return "Thrown: burning pitch that no armor stops: damage 4, 6 or 8 (grade 1, 2, 3) plus up to 6 more at random, a quarter of that further off, minus a tenth of the armor's quality (and of Firewall's protection); it also wears the armor down.";
    case 104: return "Thrown: a pool of sticky sludge (up to 4 by 4 paces) that slows whoever crosses it to 1/2, 3/8 or 1/4 of their pace (grade 1, 2, 3), for a long while.";
    case 105: if (fix_is_on("oilweapons")) return std::string("Used (P in the inventory) by a member: +3, +4 or +5 damage (grade 1, 2, 3) on every hit they land with any weapon except an impact weapon or a flail. Damage only, not the chance to hit;") + day;
              return "Used (P in the inventory) by a member: +3, +4 or +5 damage (grade 1, 2, 3) on every hit they land, with any weapon: bows, guns, impact weapons and fists too. Damage only, not the chance to hit;" + day;
    case 106: if (fix_is_on("oilweapons")) return "Used by a member: +3, +4 or +5 penetration (grade 1, 2, 3) on every hit they land with a blade or a polearm, " + std::string(fixed ? "up to 15 in all" : "past 15 the penetration starts again from 0") + ";" + day;
              return "Used by a member: +3, +4 or +5 penetration (grade 1, 2, 3) on every hit they land, with any weapon, " + std::string(fixed ? "up to 15 in all" : "past 15 the penetration starts again from 0 (a bow's 6 + 10 hits as 0)") + ";" + day;
    case 107: if (fix_is_on("greatpower")) return std::string("Used by a member: the weapon in hand counts as 14, 22 or 30 points higher in quality (grade 1, 2, 3; up to 99), ") + (fix_is_on("oilweapons") ? "if it is an impact weapon, a flail or a handgun" : "including bows, guns and fists") + ", for about a day (19 to 21 hours). Each 10 points of quality add one step of damage. A second dose adds to the first.";
              return "Used by a member, on every hit they land, with any weapon: grade 1 +14 penetration; grade 2 +1 damage and +6 penetration; grade 3 +1 damage and +14 penetration. " + std::string(fixed ? "Penetration counts up to 15 in all" : "Penetration past 15 starts again from 0, so grades 1 and 3 lower a bow's penetration") + ";" + day;
    case 108: return "Used by a member: +14, +22 or +30 (grade 1, 2, 3) to Bows, Thrown Weapons and Missile Devices (up to 99), for about a day (19 to 21 hours). A second dose adds to the first.";
    case 109: if (fix_is_on("armorday")) return "Used by a member: their armor counts 20, 25 or 30 points better in quality and 1, 2 or 2 levels thicker (grade 1, 2, 3) against every blow and shot, for about a day (19 to 21 hours). A second dose while one is active keeps the stronger.";
              return "Used by a member: +1, +2 or +3 (grade 1, 2, 3) to their armor against every hit, blow or shot, permanently; doses add up.";
    case 110: return "Drinking it yields just one florin: it is meant for cards, where an encounter asks for it (to purify, or to make gold).";
    case 111: case 112: case 113: case 114: {
        static const char *const ATT[4] = { "{Perception}", "{Endurance}", "{Strength}", "{Agility}" };
        static const char *const AMT[4] = { "9-14, 15-21 or 22-29", "6-11, 8-13 or 10-15", "6-11, 8-13 or 10-15", "6-11, 8-15 or 10-17" };
        int a = id - 111;
        std::string t = std::string("Drunk: ") + ATT[a] + " up by " + AMT[a] + " (grade 1, 2, 3) for about a day: the current value rises, past the maximum if need be; the maximum itself does not change. ";
        if (a == 1 || a == 2) t += std::string("On a wounded member it heals: ") + (fix_is_on("blessing") ? "when it wears off, only the part still above the maximum is lost, so the healing stays." : "but when it wears off the whole amount is taken back, healed or not.");
        else t += "When it wears off, what it added is lost.";
        return t + " A second dose while the first is active adds only what goes beyond it.";
    }
    case 115: return "Drunk: permanently heals 50, 70 or 90% of the {Strength} lost to wounds (grade 1, 2, 3), and raises {Endurance} by 9-14, 15-21 or 22-29 for about 30 hours.";
    case 116: {
        if (fix_is_on("armorday")) return "Drunk: protection against fire for about a day (19 to 21 hours): Arabian Fire does 5, 7 or 9 points less (grade 1, 2, 3). A second dose keeps the stronger.";
        return std::string("Drunk: permanent protection against fire: Arabian Fire does 5, 7 or 9 points less (grade 1, 2, 3).") + (fix_is_on("fireward") ? "" : " It also adds 16 to the armor permanently."); }
    }
    return "";
}

float ns_item_strip(NsDraw &P, float y0, const NsStripThing &t)
{
    const ItemDef *d = ns_itemdef(t.id);
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT;
    P.icon(71, y0 + 8, 11.0f, t.id, t.qual);
    float rw = t.right.empty() ? 0 : P.width(P.fo, NsDraw::FS_BODY, t.right) + 6;
    P.text(P.f, NsDraw::FS_HEAD, 79, y0 + 1.5f, ink, ns_item_name(t.id), -1, 314 - rw);
    if (!t.right.empty()) P.text(P.fo, NsDraw::FS_BODY, 0, y0 + 1.8f, t.right_col, t.right, 314);
    /* (the weight only of what can be worn or wielded: the load is the equipped things' alone,
       equip_weight s031 18e7:140a; what is merely carried weighs nothing) */
    bool wearable = item_wieldable(d);
    std::string l1 = std::string(d ? kind_name_one(d->kind) : "") + ns_quality_words(t.id, t.qual) + (wearable ? ", weight " + std::to_string(t.weight) + " when equipped" : std::string());
    if (t.count > 1) l1 += " each, " + std::to_string(t.count) + " of them";
    if (!t.status.empty()) l1 += "; " + t.status;
    float y = y0 + 10;
    P.text(P.fo, NsDraw::FS_BODY, 79, y, soft, l1); y += 7;
    std::string l2 = ns_item_line(t.id, t.qual);
    if (!l2.empty()) { P.text(P.fo, NsDraw::FS_BODY, 79, y, soft, l2); y += 7; }
    if (d && d->kind == K_INGREDIENT) return y + ns_ingredient_row(P, 79, y, 314, t.id);   /* an ingredient: the potions it goes into, as pictures */
    std::string use = ns_item_use(t.id);                    /* what it is good for (rope, lockpicks ...) */
    if (!use.empty()) {
        y += P.para(P.fo, NsDraw::FS_BODY, 79, y, 235, soft, use);
        ImU32 ec; std::string enc = ns_item_encounters(t.id, &ec);
        if (!enc.empty() && (t.y_max <= 0 || y + 0.5f + P.para_height(P.fo, NsDraw::FS_BODY, 235, enc) <= t.y_max))   /* (no room: the use says enough) */
            y += 0.5f + P.para(P.fo, NsDraw::FS_BODY, 79, y + 0.5f, 235, ec, enc);
    }
    else if (t.cmp_who >= 0 || t.cmp_who == -2) {          /* a weapon or an armour: against what the member (-2: the party) has now */
        std::string cmp = t.cmp_who == -2 ? ns_item_compare_party(t.id, t.qual) : ns_item_compare(t.id, t.qual, t.cmp_who);
        if (!cmp.empty()) y += P.para(P.fo, NsDraw::FS_BODY, 79, y, 235, ink, cmp) + 0.5f;
        /* the member's load were it worn instead of what is in that slot (ns_load_with), and the band
           it comes to (sheetview's bands and what they do: ui_load_effect) */
        int m = t.cmp_who, after = wearable && m >= 0 ? ns_load_with(t.id, m) : -1;
        if (after >= 0 && after != gm_member_load(m)) {
            int e, st, mx; gm_member_attr(m, 0, &e, &mx); gm_member_attr(m, 1, &st, &mx);
            int cap = e + st, cur = gm_member_load(m);
            auto band = [&](int l) { return 2 * l <= cap ? 0 : l <= cap ? 1 : 2 * l <= 3 * cap ? 2 : 3; };
            static const char *BAND[4] = { "a light load: no hindrance", "a normal load: blows hand to hand a fifth slower",
                                           "a heavy load: blows a fifth slower, dodging hampered", "overloaded: blows two thirds slower, little or no dodging" };
            int b0 = band(cur), b1 = band(after);
            char line[200];
            snprintf(line, sizeof line, "Worn by %s: load %d \xc2\xbb %d of %d, %s%s", rich_member(m).c_str(), cur, after, cap, BAND[b1],
                     b1 > b0 ? " (now " : b1 < b0 ? " (now " : "");
            std::string l = line;
            if (b1 != b0) { static const char *SHORT[4] = { "light", "normal", "heavy", "overloaded" }; l += SHORT[b0]; l += ")"; }
            ImU32 c = b1 >= 3 ? NsDraw::RED : b1 == 2 ? IM_COL32(150, 80, 20, 255) : after < cur ? NsDraw::GREEN : soft;   /* (as the heading's note: ns_load_note) */
            y += P.para(P.fo, NsDraw::FS_BODY, 79, y, 235, c, l);
        }
    }
    return y;
}

std::string ns_quality_tag(int id, int qual)
{
    char b[24] = "";
    const ItemDef *d = ns_itemdef(id);
    if (ns_shows_quality(id, qual)) snprintf(b, sizeof b, "%dq", qual);
    else if (d && d->kind == K_POTION && config_get_int("display.potion_grade", 4) == 2) {
        int ui_potion_tier(int q, int base);
        int t = ui_potion_tier(qual, d->quality);
        if (t > 0) snprintf(b, sizeof b, "grade %d", t);
    }
    return b;
}

/* the place a thing is worn or held: 1 body armour, 2 the limbs' armour, 3 shield, 4 the weapon in
   hand, 5 the missile weapon; 0 none */
int ns_slot_of(int id)
{
    const ItemDef *d = ns_itemdef(id);
    if (!d) return 0;
    if (d->type >= 67 && d->type <= 84) return d->type <= 75 ? 1 : 2;
    if (d->kind == K_SHIELD) return 3;
    if (item_weaponlike(d)) {
        int sk = rb(G_FAR20A5, (u16)(0x75ac + d->type));
        return sk == 4 || sk == 5 || sk == 6 ? 5 : 4;   /* (thrown weapons are readied as the missile weapon) */
    }
    return 0;
}
/* a ware against what member m has there now: 1 better, -1 worse, 0 alike or not to be weighed
   (another kind of weapon or shield), 2 nothing there. Armour: harder first, then better made
   (as ns_item_compare says it); a weapon or shield: better made, of the same kind */
/* member m's load were thing id worn in its slot instead of what is there (only equipped things
   weigh: equip_weight); -1 when it is not worn */
int ns_load_with(int id, int m)
{
    int sl = ns_slot_of(id);
    if (!sl || m < 0 || m >= 5 || !gm_member_present(m)) return -1;
    static const u16 TOFF[6] = { 0, 0x37, 0x38, 0x48, 0x3d, 0x0e };
    int t = (s8)g_rb((u16)(G_MEMBER(m) + TOFF[sl])), old = 0;
    if (t >= 0) for (int k = 0; k < (int)gamedata.items.size(); k++) if (gamedata.items[k].type == t) { old = gm_itemdef_weight(k); break; }
    int v = gm_member_load(m) - old + gm_itemdef_weight(id);
    return v < 0 ? 0 : v;
}
/* "Load 42 of 76", or with a thing pointed at "Load 42 > 47 of 76", right-aligned to x1 on a heading
   line at y, coloured by the band it comes to; its width (the heading's rule stops short of it) */
float ns_load_note(NsDraw &P, float x1, float y, int m, int after)
{
    if (m < 0 || m >= 5 || !gm_member_present(m)) return 0;
    int e, s, mx; gm_member_attr(m, 0, &e, &mx); gm_member_attr(m, 1, &s, &mx);
    int cap = e + s, cur = gm_member_load(m), v = after >= 0 ? after : cur;
    auto band = [&](int l) { return 2 * l <= cap ? 0 : l <= cap ? 1 : 2 * l <= 3 * cap ? 2 : 3; };
    int b = band(v);
    ImU32 c = b >= 3 ? NsDraw::RED : b == 2 ? IM_COL32(150, 80, 20, 255) : NsDraw::INK_SOFT;
    char t[64];
    if (after >= 0 && after != cur) snprintf(t, sizeof t, "Load %d \xc2\xbb %d of %d", cur, after, cap);
    else snprintf(t, sizeof t, "Load %d of %d", cur, cap);
    if (after >= 0 && after != cur && b == band(cur)) c = after < cur ? NsDraw::GREEN : NsDraw::INK;   /* (the same band: lighter green, heavier ink) */
    float w = P.width(P.fo, NsDraw::FS_SMALL, t);
    P.text(P.fo, NsDraw::FS_SMALL, 0, y, c, t, x1);
    return w;
}

int ns_against_worn(int id, int q, int m)
{
    int sl = ns_slot_of(id);
    if (!sl || m < 0 || m >= 5 || !gm_member_present(m)) return 0;
    static const u16 TOFF[6] = { 0, 0x37, 0x38, 0x48, 0x3d, 0x0e }, QOFF[6] = { 0, 0x3b, 0x3c, 0x47, 0x44, 0x46 };
    u16 b = G_MEMBER(m);
    int t = (s8)g_rb((u16)(b + TOFF[sl])), wq = g_rb((u16)(b + QOFF[sl]));
    if (t < 0) return 2;
    const ItemDef *d = ns_itemdef(id);
    if (sl <= 2) {
        if (t < 67 || t > 84) return 2;
        int h = rb(G_FAR20A5, (u16)(0x781e + d->type)), wh = rb(G_FAR20A5, (u16)(0x781e + t));
        if (h != wh) return h > wh ? 1 : -1;
        return q > wq ? 1 : q < wq ? -1 : 0;
    }
    if (sl >= 4 && t > 62) return 2;
    if (t != d->type) return 0;
    return q > wq ? 1 : q < wq ? -1 : 0;
}


void ns_compare_arrow(ImDrawList *dl, float ax, float ay, int vs, float k)
{
    if (!vs) return;
    ImU32 ac = vs == 1 ? NsDraw::GREEN : vs == 2 ? IM_COL32(40, 96, 34, 170) : NsDraw::RED;
    ImVec2 p0 = NsDraw::w(ax - 2, vs < 0 ? ay - 1.4f : ay + 1.4f), p1 = NsDraw::w(ax + 2, vs < 0 ? ay - 1.4f : ay + 1.4f), p2 = NsDraw::w(ax, vs < 0 ? ay + 1.8f : ay - 1.8f);
    if (vs == 2) dl->AddTriangle(p0, p1, p2, ac, 0.3f * k); else dl->AddTriangleFilled(p0, p1, p2, ac);
}

/* the section of the items list a thing goes in (the character sheet, the shops): 0 equipped (the
   caller's), 1 potions, 2 spare equipment (weapons, armour, shields, mounts), 3 utility (rope,
   lockpicks, instruments, tools ...), 4 trophies and quest things (relics, documents), 5 ingredients */
int ns_item_section(int id)
{
    const ItemDef *d = ns_itemdef(id);
    if (!d) return 3;
    switch (d->kind) {
    case K_POTION: return 1;
    case K_WEAPON: case K_MISSILE: case K_ARMOR: case K_SHIELD: case K_HORSE: return 2;
    case K_RELIC: case K_DOCUMENT: case K_QUEST: return 4;
    case K_INGREDIENT: return 5;
    default: return 3;
    }
}
const char *ns_item_section_name(int sec)
{
    static const char *N[6] = { "Equipped", "Potions", "Spare equipment", "Tools and other goods", "Trophies and quest items", "Ingredients" };
    return sec >= 0 && sec < 6 ? N[sec] : "";
}

/* ---------------------------------------------------------------- a merchant's counter
   The screens where things change hands (the shop, the store room): the back wall of dark timber
   with an awning's scalloped edge along the top, the oak counter below it (the alchemy desk's
   table), the lists laid on it as sheets of paper with their shadows (ns_sheet). Drawn once and
   kept (ui/drawcache.h). */
#include "alchemydesk.h"
#include "drawcache.h"
int ns_page_frame = -10;                 /* the frame NsDraw::page was last drawn */
void ns_counter_backdrop(NsDraw &P, float wall_to, ImU32 awning, bool stone)
{
    ImVec2 o = NsDraw::w(60, 0), e = NsDraw::w(320, 200);
    unsigned long long key = 0xc0417e5ull;
    key = dc_key(key, wall_to); key = dc_key(key, (double)awning); key = dc_key(key, stone);
    dc_draw(P.dl, o, e, key, [&](ImDrawList *dl) {
        desk_table(dl);
        /* the wall: vertical boards, darker, with their seams, the counter's front edge under it */
        ImVec2 a = NsDraw::w(60, 0), b = NsDraw::w(320, wall_to);
        dl->AddRectFilledMultiColor(a, b, IM_COL32(58, 36, 20, 255), IM_COL32(52, 32, 18, 255), IM_COL32(40, 24, 12, 255), IM_COL32(44, 26, 14, 255));
        if (stone) {                                           /* a cellar's wall: courses of stone blocks, lit from the upper left */
            dl->AddRectFilledMultiColor(a, b, IM_COL32(96, 90, 80, 255), IM_COL32(86, 80, 72, 255), IM_COL32(58, 54, 48, 255), IM_COL32(64, 60, 54, 255));
            for (int r = 0; r * 7.5f < wall_to; r++) {
                float y0 = r * 7.5f, y1 = fminf(wall_to, y0 + 7.5f);
                for (int c = -1; c < 14; c++) {
                    float x0 = 60 + c * 20 + (r & 1) * 10 + (float)((r * 7 + c * 13) % 5), x1 = x0 + 19.4f;
                    unsigned h = (unsigned)(r * 131 + c * 71) * 2654435761u; int v = (int)((h >> 24) % 30) - 15;
                    dl->AddRectFilled(NsDraw::w(fmaxf(60, x0 + 0.6f), y0 + 0.6f), NsDraw::w(fminf(320, x1), y1 - 0.4f), IM_COL32(110 + v, 104 + v, 92 + v, 120), 1.2f * P.k);
                    dl->AddLine(NsDraw::w(fmaxf(60, x0 + 0.6f), y0 + 0.8f), NsDraw::w(fminf(320, x1), y0 + 0.8f), IM_COL32(170, 160, 140, 70), 1.0f);
                }
                dl->AddLine(NsDraw::w(60, y1), NsDraw::w(320, y1), IM_COL32(30, 26, 22, 200), 1.2f);
            }
        } else
        for (int i = 0; i < 18; i++) {
            float x = 60 + i * 14.6f + (i * 37 % 5);
            dl->AddLine(NsDraw::w(x, 0), NsDraw::w(x, wall_to), IM_COL32(20, 10, 4, 200), 1.4f);
            dl->AddLine(NsDraw::w(x + 0.6f, 0), NsDraw::w(x + 0.6f, wall_to), IM_COL32(110, 74, 40, 60), 1.0f);
        }
        dl->AddRectFilledMultiColor(NsDraw::w(60, wall_to - 1.2f), NsDraw::w(320, wall_to + 2.2f), IM_COL32(120, 78, 40, 255), IM_COL32(120, 78, 40, 255), IM_COL32(60, 36, 16, 255), IM_COL32(60, 36, 16, 255));
        dl->AddRectFilledMultiColor(NsDraw::w(60, wall_to + 2.2f), NsDraw::w(320, wall_to + 6), IM_COL32(0, 0, 0, 110), IM_COL32(0, 0, 0, 110), 0, 0);
        /* the awning's edge: scallops in two colours */
        if (awning) {
            ImU32 alt = IM_COL32(236, 224, 196, 255);
            for (int i = 0; i < 26; i++) {
                float x0 = 60 + i * 10, xm = x0 + 5;
                ImU32 c = i & 1 ? alt : awning;
                dl->AddRectFilled(NsDraw::w(x0, 0), NsDraw::w(x0 + 10, 1.4f), c);
                dl->PathClear(); dl->PathLineTo(NsDraw::w(x0, 1.4f));
                for (int k = 0; k <= 8; k++) { float t = 3.14159f * k / 8; dl->PathLineTo(NsDraw::w(xm - cosf(t) * 5, 1.4f + sinf(t) * 2.2f)); }
                dl->PathFillConvex(c);
            }
        }
    });
}
/* a sheet of paper lying on the counter: its shadow, the paper (then the list is written on it) */
void ns_sheet(NsDraw &P, float x0, float y0, float x1, float y1, ImU32 paper)
{
    P.dl->AddRectFilled(NsDraw::w(x0 + 1.2f, y0 + 1.8f), NsDraw::w(x1 + 1.6f, y1 + 2.2f), IM_COL32(0, 0, 0, 90), 2 * P.k);
    P.dl->AddRectFilled(NsDraw::w(x0, y0), NsDraw::w(x1, y1), paper, 1.5f * P.k);
    P.dl->AddRectFilledMultiColor(NsDraw::w(x0, y1 - 4), NsDraw::w(x1, y1), 0, 0, IM_COL32(120, 90, 50, 40), IM_COL32(120, 90, 50, 40));
}

/* the party's pack: a leather pack with its flap, buckle and shoulder strap (drawn, as the order and
   skill pictures, so it keeps sharp at any size) */
void ns_backpack_icon(NsDraw &P, float cx, float cy, float s)
{
    ImDrawList *dl = P.dl;
    auto W = [&](float x, float y) { return NsDraw::w(cx + x * s * P.k / P.kx, cy + y * s); };   /* (square on screen: a column is narrower than a row) */
    float r = 0.12f * s * P.k;
    const ImU32 strap = IM_COL32(92, 52, 22, 255), body = IM_COL32(150, 92, 44, 255), flap = IM_COL32(118, 68, 30, 255),
                edge = IM_COL32(60, 34, 14, 255), gold = IM_COL32(214, 172, 74, 255);
    dl->AddBezierCubic(W(-0.22f, -0.30f), W(-0.22f, -0.58f), W(0.22f, -0.58f), W(0.22f, -0.30f), strap, fmaxf(1.0f, 0.09f * s * P.k));   /* (the carrying loop) */
    dl->AddRectFilled(W(-0.40f, -0.34f), W(0.40f, 0.50f), body, r);                      /* (the bag) */
    dl->AddRectFilled(W(-0.40f, 0.18f), W(0.40f, 0.50f), IM_COL32(132, 80, 36, 255), r, ImDrawFlags_RoundCornersBottom);   /* (its pocket) */
    dl->AddRect(W(-0.40f, -0.34f), W(0.40f, 0.50f), edge, r, 0, fmaxf(1.0f, 0.05f * s * P.k));
    dl->AddRectFilled(W(-0.42f, -0.36f), W(0.42f, 0.06f), flap, r, ImDrawFlags_RoundCornersAll);   /* (the flap over it) */
    dl->AddRect(W(-0.42f, -0.36f), W(0.42f, 0.06f), edge, r, 0, fmaxf(1.0f, 0.05f * s * P.k));
    dl->AddRectFilled(W(-0.05f, -0.06f), W(0.05f, 0.30f), strap);                        /* (the strap down the flap) */
    dl->AddRect(W(-0.10f, 0.02f), W(0.10f, 0.16f), gold, 0, 0, fmaxf(1.0f, 0.05f * s * P.k));   /* (its buckle) */
}
