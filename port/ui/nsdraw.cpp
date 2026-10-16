/* nsdraw.cpp - see nsdraw.h */
#include "nsdraw.h"
#include "../runtime/cpu.h"
#include "../runtime/game.h"

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
