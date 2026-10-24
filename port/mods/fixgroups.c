/* fixgroups.c - the fixes (fixes.c) sorted by kind, for the settings' list: from what the game
 * plainly gets wrong to what is open to argument, as players have talked of them over the years
 * (the GOG and Steam forums, the old Usenet posts and the patch notes MicroProse left). Each fix
 * stays its own switch; this only orders and heads them. A fix not listed here falls under
 * "Plain mistakes". */
#include <string.h>
#include "fixgroups.h"

const FixGroup FIX_GROUPS[FIX_NGROUPS] = {
    { "Game-breaking", "Crashes, freezes, items or progress lost, saved games spoiled, quests that can never be finished. Nobody wants these." },
    { "Plain mistakes", "Something the game sets out to do and gets wrong: the wrong member, the wrong town, a check on the wrong option, a number lost on the way." },
    { "As the manual and the game's texts say", "The game does something other than what its manual, or its own texts, say it does. Fixed to match the words." },
    { "Exploits closed", "Bugs players have long used to their advantage (invulnerability from fire protection, confession's clock turned back, the same bounty again and again). Leave these off to keep the old tricks." },
    { "Not a bug", "Left over from 1992, and nobody misses it." },
};

static const struct { const char *id; int g; } G[] = {
    /* game-breaking */
    { "nullitem", 0 }, { "itemcount", 0 }, { "testcard", 0 }, { "fullpack", 0 }, { "castlefreeze", 0 }, { "lootcount", 0 },
    { "innsave", 0 }, { "offmap", 0 }, { "altar", 0 }, { "castle", 0 }, { "armorwear", 0 }, { "potions", 0 }, { "standings", 0 },
    { "battledead", 0 }, { "blessing", 0 }, { "colors", 0 },
    /* as the manual and the texts say */
    { "greatpower", 2 }, { "oilweapons", 2 }, { "armorday", 2 }, { "pauls", 2 }, { "sainttexts", 2 }, { "teachcap", 2 }, { "alchrisk", 2 },
    /* exploits */
    { "fireward", 3 }, { "confession", 3 }, { "rkonce", 3 }, { "bounty", 3 },
    /* not a bug */
    { "copyprot", 4 },
};

int fix_group(const char *id)
{
    for (size_t i = 0; i < sizeof G / sizeof G[0]; i++) if (!strcmp(G[i].id, id)) return G[i].g;
    return 1;
}
