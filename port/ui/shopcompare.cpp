/* shopcompare.cpp - the shop's party-wide upgrade marks and the members' comparisons under the wares. */
#include "shopcompare.h"
#include "nsdraw.h"
#include "../mods/gamemodel.h"
#include "../mods/relics.h"
#include "../runtime/game.h"
#include <algorithm>

namespace {
enum class Fit { Better, Empty, Worse, Even, Mixed, Alternative, Blocked };
struct Comparison { Fit fit; const char *label; };

bool shopping_member(int m)
{
    int status = gm_member_status(m);
    return gm_member_present(m) && status != 3 && status != 5;
}

int stat(int base, int type) { return rb(G_FAR20A5, (u16)(base + type)); }
bool two_handed(int type) { return type >= 0 && type <= 62 && (stat(0x76a8, type) >> 4) >= 2; }

/* A different weapon trades attack, penetration, speed and sometimes a shield or a special
   effect. Give the member's change in melee rating or aim (weapon_ready) without promising
   that a higher rating alone makes one weapon better. */
int weapon_rating(int type, int m, bool missile)
{
    int current, strength;
    gm_member_attr(m, ATTR_STR, &current, &strength);  /* weapon_ready uses combatant +0x65 (maximum STR). */
    int skill = gm_member_skill(m, stat(0x75ac, type));
    int short_skill = std::max(0, stat(0x7726, type) - skill);
    int short_str = std::max(0, stat(0x7765, type) - strength);
    int bonus = missile ? 0 : 2 * std::max(0, strength - stat(0x77a4, type));
    return std::max(0, std::min(255, skill + bonus - 2 * short_skill - (missile ? 1 : 3) * short_str));
}

/* Only rank different weapons directly when they use the same skill, hands, ammunition
   and item flags. A gain on one measure with a loss elsewhere stays an alternative. */
Comparison compare_weapons(int id, int q, int m, int slot, NsWorn old)
{
    const ItemDef &ware = *ns_itemdef(id);
    const ItemDef *equipped = ns_itemdef(ns_worn_id(m, old));
    int w = ware.type, t = old.type;
    const Comparison alternative = { Fit::Alternative, "Different weapon" };
    if (!equipped || ware.kind == K_RELIC || equipped->kind == K_RELIC || ware.cats() != equipped->cats()
        || stat(0x75ac, w) != stat(0x75ac, t) || (stat(0x76a8, w) >> 4) != (stat(0x76a8, t) >> 4)
        || (slot == 5 && stat(0x7822, w) != stat(0x7822, t))) return alternative;

    /* A lower minimum or maximum Strength is favorable in hit_damage (s014).
       Count a clear benefit only on an actual combat stat, quality or weight. */
    int delta[] = {
        weapon_rating(w, m, slot == 5) - weapon_rating(t, m, slot == 5),
        stat(0x76e7, w) - stat(0x76e7, t), (stat(0x76a8, w) & 15) - (stat(0x76a8, t) & 15),
        stat(0x7669, t) - stat(0x7669, w), stat(0x77e3, t) - stat(0x77e3, w),
        q - old.quality, equipped->weight - ware.weight
    };
    bool gain = false, loss = false;
    for (int d : delta) { gain |= d > 0; loss |= d < 0; }
    if (gain && !loss && stat(0x7765, w) <= stat(0x7765, t) && stat(0x77a4, w) <= stat(0x77a4, t))
        return { Fit::Better, "Better weapon" };
    if (loss && !gain && stat(0x7765, w) >= stat(0x7765, t) && stat(0x77a4, w) >= stat(0x77a4, t))
        return { Fit::Worse, "Worse weapon" };
    return alternative;
}

Comparison compare(int id, int q, int m, int slot, NsWorn old)
{
    const int type = ns_itemdef(id)->type;
    if (relics_on() && relic_is_weapon(id) && !relic_bearer_ok(m, id)) return { Fit::Blocked, "Needs Virtue" };
    if (slot == 3 && two_handed(ns_worn_slot(m, 4).type)) return { Fit::Blocked, "Needs free hand" };
    if (slot == 4 && two_handed(type) && !two_handed(old.type) && ns_worn_slot(m, 3).type >= 0)
        return { Fit::Mixed, "Shield inactive" };
    if (old.type < 0 || (slot >= 4 && old.type > 62)) return { Fit::Empty, "Fills empty slot" };
    if (slot <= 2 && old.type >= 67 && old.type <= 84) {
        int diff = stat(0x781e, type) - stat(0x781e, old.type);
        if (diff > 0) return { Fit::Better, "Harder armor" };
        if (diff < 0) return { Fit::Worse, "Softer armor" };
    } else if (slot == 3) {
        int melee = gm_shield_defence(type, q, 0) - gm_shield_defence(old.type, old.quality, 0);
        int shot = gm_shield_defence(type, q, 1) - gm_shield_defence(old.type, old.quality, 1);
        if ((melee > 0 && shot < 0) || (melee < 0 && shot > 0)) return { Fit::Mixed, "Mixed defense" };
        if (melee > 0 || shot > 0) return { Fit::Better, "More defense" };
        if (melee < 0 || shot < 0) return { Fit::Worse, "Less defense" };
        return { Fit::Even, "Same defense" };
    } else if (slot >= 4 && type != old.type) return compare_weapons(id, q, m, slot, old);
    if (q > old.quality) return { Fit::Better, "Better quality" };
    if (q < old.quality) return { Fit::Worse, "Lower quality" };
    return { Fit::Even, slot <= 2 ? "Same protection" : "Same weapon" };
}

std::string signed_value(int n) { return (n > 0 ? "+" : "") + std::to_string(n); }
}

int shop_upgrade_count(int id, int q, bool &only_empty)
{
    int slot = ns_slot_of(id), count = 0;
    only_empty = true;
    if (!slot) return 0;
    for (int m = 0; m < gm_party_size() && m < 5; m++) {
        if (!shopping_member(m)) continue;
        Fit fit = compare(id, q, m, slot, ns_worn_slot(m, slot)).fit;
        if (fit == Fit::Better) only_empty = false;
        if (fit == Fit::Better || fit == Fit::Empty) count++;
    }
    return count;
}

void shop_compare_strip(NsDraw &p, float y, const NsStripThing &ware)
{
    const int slot = ns_slot_of(ware.id);
    const int type = ns_itemdef(ware.id)->type;
    p.icon(71, y + 7, 10, ware.id, ware.qual);
    float price_x = 314 - p.width(p.fo, NsDraw::FS_BODY, ware.right);
    p.text(p.f, NsDraw::FS_HEAD, 79, y + 1, NsDraw::INK, ns_item_name(ware.id) + ns_quality_words(ware.id, ware.qual), -1, price_x - 3);
    p.text(p.fo, NsDraw::FS_BODY, 0, y + 2, ware.right_col, ware.right, 314);
    p.text(p.fo, NsDraw::FS_SMALL, 79, y + 9, NsDraw::INK_SOFT, ns_item_line(ware.id, ware.qual) + "  ·  Weight " + std::to_string(ware.weight), -1, 314);
    p.rect(66, y + 16, 314, y + 16.3f, IM_COL32(130, 94, 51, 125));

    int count = 0;
    for (int m = 0; m < 5; m++) if (shopping_member(m)) count++;
    float x = 66, width = 248.0f / std::max(count, 1);
    for (int m = 0; m < 5; m++) {
        if (!shopping_member(m)) continue;
        float end = x + width - 2;
        NsWorn old = ns_worn_slot(m, slot);
        Comparison match = compare(ware.id, ware.qual, m, slot, old);
        const bool up = match.fit == Fit::Better || match.fit == Fit::Empty;
        ImU32 color = up ? NsDraw::GREEN : match.fit == Fit::Worse || match.fit == Fit::Blocked ? NsDraw::RED : NsDraw::INK_SOFT;
        if (x > 66) p.rect(x - 1.3f, y + 18, x - 1, y + 44, IM_COL32(130, 94, 51, 110));
        ui_member_head(p.dl, m, p.w(x, y + 17), 7.8f * p.k);
        p.text(p.f, NsDraw::FS_NAME, x + 8, y + 17, NsDraw::INK, ui_member_name(m), -1, end);
        int old_id = ns_worn_id(m, old);
        std::string previous = old_id >= 0 ? ns_item_name(old_id) + " " + std::to_string(old.quality) + "q" : "Empty slot";
        p.text(p.fo, NsDraw::FS_SMALL, x, y + 24.5f, NsDraw::INK_SOFT, previous, -1, end);
        std::string verdict = match.label;
        if (match.fit == Fit::Alternative) {
            int diff = weapon_rating(type, m, slot == 5) - weapon_rating(old.type, m, slot == 5);
            verdict = std::string(slot == 5 ? "Aim " : "Melee rating ") + signed_value(diff);
        }
        int arrow = up ? (match.fit == Fit::Empty ? 2 : 1) : match.fit == Fit::Worse ? -1 : 0;
        if (arrow) ns_compare_arrow(p.dl, x + 2.2f, y + 33.5f, arrow, p.k);
        p.text(p.fo, NsDraw::FS_BODY, x + (arrow ? 5.8f : 0), y + 30.2f, color, verdict, -1, end);

        int cur = gm_member_load(m), after = ns_load_with(ware.id, m);
        int e, s, mx;
        gm_member_attr(m, ATTR_END, &e, &mx);
        gm_member_attr(m, ATTR_STR, &s, &mx);
        int cap = e + s;
        std::string load = "Load " + signed_value(after - cur) + " · " + std::to_string(after) + "/" + std::to_string(cap);
        ImU32 burden = after * 2 > 3 * cap ? NsDraw::RED : after > cap ? IM_COL32(144, 72, 18, 255) : NsDraw::INK;
        p.text(p.fo, NsDraw::FS_SMALL, x, y + 37.8f, burden, load, -1, end);
        if (p.over(x, y + 17, end, y + 44.5f)) {
            std::string note = ui_member_name(m) + "\nEquipped: " + previous + "\n" + match.label + ". ";
            if (old_id >= 0) note += ns_item_line(old_id, old.quality) + ". ";
            if (slot >= 4) note += "With this weapon: " + std::to_string(weapon_rating(type, m, slot == 5)) + (slot == 5 ? " aim. " : " melee rating. ");
            note += "\nLoad " + std::to_string(cur) + " to " + std::to_string(after) + " of " + std::to_string(cap);
            note += after * 2 <= cap ? " (light)." : after <= cap ? " (normal)." : after * 2 <= 3 * cap ? " (heavy)." : " (overloaded).";
            ns_tip(note.c_str());
        }
        x += width;
    }
}
