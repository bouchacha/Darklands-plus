// libraries.cpp - see libraries.h.
//
// Which places have a library the party can study in: the saint card ($Learn00, card 0x39) is
// reached from a town's monastery ($CityM00), its university ($Unive00) and a monastery in the
// country ($Count04); each draws its saints with srand(party seed + the place). So: the towns
// with a monastery (DARKLAND.CTY building bit 7) or a university (bit 8), and the locations of the
// monastery kind (3).
#include <math.h>
#include <algorithm>
#include "libraries.h"
#include "gamedata.h"
#include "quests.h"
#include "townicons.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../mods/gamemodel.h"
#include "../runtime/config.h"
int journal_visited(int loc);
}

static std::vector<Library> libs;
static int libs_seed = -100000;

const std::vector<Library> &libraries(void)
{
    int gm = game_mode();
    if (gm != GM_CARD && gm != GM_MAP) return libs;               /* (battles reuse the location table) */
    int seed = (s16)g_rw(G_PARTY_SEED);
    if (seed == libs_seed && !libs.empty()) return libs;
    libs.clear();
    for (int k = 0; k < G_NLOC; k++) {
        u32 r = g_location(k);
        if (!r) continue;
        u16 s = (u16)(r >> 16), o = (u16)r;
        int kind = (s16)rw(s, o);
        bool lib;
        if (k < (int)gamedata.cities.size()) lib = (gamedata.cities[k].buildings >> 7 & 1) || (gamedata.cities[k].buildings >> 8 & 1);
        else lib = kind == 3;
        if (!lib) continue;
        Library L;
        L.loc = k; L.x = (s16)rw(s, (u16)(o + 4)); L.y = (s16)rw(s, (u16)(o + 6));
        L.name = loc_display_name(k);
        gm_library_saints(k, L.saint);
        libs.push_back(L);
    }
    libs_seed = seed;
    return libs;
}

bool place_offers_known(int loc)
{
    if (config_get_int("map.offers_unvisited", 0)) return true;
    if (game_mode() == GM_CARD && loc == (s16)g_rw(G_CUR_LOC)) return true;
    return journal_visited(loc) != 0;
}

const Library *library_at(int loc)
{
    for (const Library &L : libraries()) if (L.loc == loc) return &L;
    return nullptr;
}

bool party_knows_saint(int s)
{
    int n = gm_party_size();
    for (int m = 0; m < n && m < 5; m++) if (gm_member_present(m) && gm_knows_saint(m, s)) return true;
    return false;
}

bool party_knows_formula(int f)
{
    int n = gm_party_size();
    for (int m = 0; m < n && m < 5; m++) if (gm_member_present(m) && gm_knows_formula(m, f)) return true;
    return false;
}

std::vector<LibraryHit> libraries_teaching(int s)
{
    std::vector<LibraryHit> v;
    double px = 0, py = 0; bool here = party_xy(&px, &py);
    for (const Library &L : libraries()) {
        bool t = false;
        for (int k = 0; k < 4; k++) if (L.saint[k] == s) t = true;
        if (!t || !place_offers_known(L.loc)) continue;
        double d = -1;
        if (here) { double dx = L.x - px, dy = (L.y - py) / 2.0; d = hypot(dx, dy) / 10.0; }   /* (about ten columns a day on foot: ui/quests.cpp) */
        v.push_back({ &L, d });
    }
    std::sort(v.begin(), v.end(), [](const LibraryHit &a, const LibraryHit &b) { return a.days < b.days; });
    return v;
}

bool alchemist_formulae(int town, int formula[4], int price_fl[4])
{
    if (town < 0 || town >= (int)gamedata.cities.size() || !citynav_town_offers(town, TI_ALCHEMIST)) return false;
    const City &c = gamedata.cities[town];
    s16 q = (s16)(c.seed + (s16)g_rw(G_PARTY_SEED));        /* loc_query 0x21: 16-bit, as the game adds them */
    int size = c.size > 0 ? c.size : 1;                      /* loc_query 2 */
    for (int k = 0; k < 4; k++) {
        int v = (q + 100 + 3 * k) % 22;                      /* (C's %, as the game's idiv) */
        formula[k] = v;
        price_fl[k] = ((v % 3) + 1) * 10 / size;
    }
    return true;
}

std::vector<SellerHit> formula_sellers(int f)
{
    std::vector<SellerHit> v;
    if (f < 0 || f >= 22) return v;
    double px = 0, py = 0; bool here = party_xy(&px, &py);
    for (int t = 0; t < (int)gamedata.cities.size() && t < 92; t++) {
        int fo[4], pr[4];
        if (!place_offers_known(t) || !alchemist_formulae(t, fo, pr)) continue;
        for (int k = 0; k < 4; k++) if (fo[k] == f) {
            double d = -1;
            if (here) { double dx = gamedata.cities[t].x - px, dy = (gamedata.cities[t].y - py) / 2.0; d = hypot(dx, dy) / 10.0; }
            v.push_back({ t, gamedata.cities[t].name, d, pr[k] });
            break;
        }
    }
    std::sort(v.begin(), v.end(), [](const SellerHit &a, const SellerHit &b) { return a.days < b.days; });
    return v;
}
extern "C" int ui_alchemist_formulae_c(int town, int *f, int *p) { return alchemist_formulae(town, f, p) ? 1 : 0; }   /* (the test command ualc) */
