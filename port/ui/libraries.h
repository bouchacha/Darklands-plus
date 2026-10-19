// libraries.h - where each saint can be learnt (ui/libraries.cpp). A monastery's or a university's
// library teaches four saints, drawn from the party's seed and the place (mods/gamemodel.c
// gm_library_saints, as the card $Learn00 does), so they are fixed for the whole game: Darklands+
// can say which library teaches a saint the party lacks, and what a town's library teaches.
#pragma once
#include <string>
#include <vector>

struct Library {
    int loc;                 // the place (a town with a monastery or a university, or a monastery)
    std::string name;
    int x, y;                // its tile on the travel map
    int saint[4];            // the saints it teaches (DARKLAND.LST order; two may be the same)
};

/* whether the party knows what place loc offers (the saints its library teaches, the formulae its
   alchemist sells, the goods its shops will have): it is there or has been there (the journal's
   record), or the switch "What unvisited places offer" (map.offers_unvisited: off, but in the Easy
   preset) is on. The lists below name only such places. */
bool place_offers_known(int loc);

/* the libraries of this game (read while a card or the travel map is up, kept otherwise) */
const std::vector<Library> &libraries(void);
const Library *library_at(int loc);                         // nullptr: no library there
/* the libraries that teach saint s, of the places whose offers the party knows, the nearest first;
   the distance in days on foot (about), -1 unknown */
struct LibraryHit { const Library *lib; double days; };
std::vector<LibraryHit> libraries_teaching(int s);
bool party_knows_saint(int s);                              // anyone in the party
bool party_knows_formula(int f);                            // anyone in the party

/* the formulae a town's alchemist sells: four, fixed for the whole game ($Alche01, s064 1b0b: the
   town's seed + the party's + 100 + 3 a slot, % 22: only the first 22 formulae, DARKLAND.LST order,
   are ever sold), with their prices in florins. False: the town has no alchemist. */
bool alchemist_formulae(int town, int formula[4], int price_fl[4]);
/* the towns whose alchemist sells formula f, of those whose offers the party knows, the nearest
   first (days -1: unknown) */
struct SellerHit { int town; std::string name; double days; int price_fl; };
std::vector<SellerHit> formula_sellers(int f);
