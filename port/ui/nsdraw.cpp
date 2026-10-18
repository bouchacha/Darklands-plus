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
               " against shots and throws; not with a two-handed weapon";
    }
    if (d->type >= 0 && d->type <= 33 && (d->kind == K_WEAPON || d->kind == K_MISSILE)) {
        int w = d->type, sk = rb(G_FAR20A5, (u16)(0x75ac + w)), cp = rb(G_FAR20A5, (u16)(0x76a8 + w)), dmg = rb(G_FAR20A5, (u16)(0x76e7 + w));
        static const char *SK[7] = { "Edged", "Impact", "Flail", "Polearm", "Thrown", "Bow", "Missile device" };
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
            r += " In hand now: " + name_of(t, wq) + " (damage " + std::to_string(dmg) + ", penetration " + std::to_string(cp & 15) + ", " + std::to_string(wq) + "q).";
        } else r += missile ? " No bow or crossbow ready now." : " Nothing in hand now.";
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
        if (!heavy.empty()) r += " Too heavy for " + join(heavy) + " (it needs strength " + std::to_string(smin) + ").";
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

void ns_item_strip(NsDraw &P, float y0, const NsStripThing &t)
{
    const ItemDef *d = ns_itemdef(t.id);
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT;
    P.icon(71, y0 + 8, 11.0f, t.id, t.qual);
    float rw = t.right.empty() ? 0 : P.width(P.fo, NsDraw::FS_BODY, t.right) + 6;
    P.text(P.f, NsDraw::FS_HEAD, 79, y0 + 1.5f, ink, ns_item_name(t.id), -1, 314 - rw);
    if (!t.right.empty()) P.text(P.fo, NsDraw::FS_BODY, 0, y0 + 1.8f, t.right_col, t.right, 314);
    std::string l1 = std::string(d ? kind_name_one(d->kind) : "") + ns_quality_words(t.id, t.qual) + ", weight " + std::to_string(t.weight);
    if (t.count > 1) l1 += " each, " + std::to_string(t.count) + " of them";
    if (!t.status.empty()) l1 += "; " + t.status;
    float y = y0 + 10;
    P.text(P.fo, NsDraw::FS_BODY, 79, y, soft, l1); y += 7;
    std::string l2 = ns_item_line(t.id, t.qual);
    if (!l2.empty()) { P.text(P.fo, NsDraw::FS_BODY, 79, y, soft, l2); y += 7; }
    if (d && d->kind == K_INGREDIENT) {                     /* an ingredient: the potions it goes into, as pictures */
        std::vector<int> pot; std::vector<bool> kn;
        for (int f = 0; f < (int)gamedata.formulas.size(); f++) {
            bool uses = false;
            for (const FormulaIngr &g : gamedata.formulas[f].ingr) if (g.item == t.id) uses = true;
            if (!uses) continue;
            int potion = 95 + f / 3; size_t k = 0;
            while (k < pot.size() && pot[k] != potion) k++;
            if (k == pot.size()) { pot.push_back(potion); kn.push_back(false); }
            for (int m = 0; m < gm_party_size() && m < 5; m++) if (gm_member_present(m) && gm_knows_formula(m, f)) kn[k] = true;
        }
        if (pot.empty()) { P.text(P.fo, NsDraw::FS_BODY, 79, y, soft, "No formula uses it."); return; }
        const char *lab = "Goes into";
        P.text(P.fo, NsDraw::FS_SMALL, 79, y + 3.0f, soft, lab);
        float x = 79 + P.width(P.fo, NsDraw::FS_SMALL, lab) + 3, sz = 10.0f, step = 11.0f;
        int hov = -1;
        for (size_t k = 0; k < pot.size(); k++) {
            float cx = x + k * step + sz * 0.5f, cy = y + sz * 0.5f;
            if (cx + sz * 0.5f > 314) break;
            P.icon(cx, cy, sz, pot[k]);
            if (!kn[k]) P.rect(cx - sz * 0.55f, cy - sz * 0.62f, cx + sz * 0.55f, cy + sz * 0.62f, IM_COL32(230, 214, 178, 185));   /* (no formula known: washed out) */
            if (P.over(cx - step * 0.5f, cy - sz * 0.6f, cx + step * 0.5f, cy + sz * 0.6f)) hov = (int)k;
        }
        std::string line;
        if (hov >= 0) line = ns_item_name(pot[hov]) + (kn[hov] ? ": the party knows a formula with it." : ": nobody in the party knows a formula with it.");
        else {
            int nk = 0; for (bool b : kn) nk += b;
            line = nk ? "Bright: the party knows a formula with it. Point at one for its name." : "Nobody in the party knows a formula with it. Point at one for its name.";
        }
        P.text(P.fo, NsDraw::FS_SMALL, 79, y + sz + 2.0f, hov >= 0 ? ink : NsDraw::INK_DIM, line, -1, 314);
        return;
    }
    std::string use = ns_item_use(t.id);                    /* what it is good for (rope, lockpicks ...) */
    if (!use.empty()) P.para(P.fo, NsDraw::FS_BODY, 79, y, 235, soft, use);
    else if (t.cmp_who >= 0 || t.cmp_who == -2) {          /* a weapon or an armour: against what the member (-2: the party) has now */
        std::string cmp = t.cmp_who == -2 ? ns_item_compare_party(t.id, t.qual) : ns_item_compare(t.id, t.qual, t.cmp_who);
        if (!cmp.empty()) P.para(P.fo, NsDraw::FS_BODY, 79, y, 235, ink, cmp);
    }
}
