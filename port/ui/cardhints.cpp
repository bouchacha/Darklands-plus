// cardhints.cpp - why an option of a card is greyed: the cards' scripts (src/c) show some
// options without letting them be chosen - something done already, the time of day, a fact the
// party has yet to learn, money - and the game never says which. One line for each such option,
// read from the scripts; the card view shows it when the pointer rests on the greyed option.
// (Options greyed because nobody knows the saint or carries the potion are explained by the
// card view itself, from the party.)
#include <strings.h>

struct CardHint { const char *card; int opt; const char *why; };

static const CardHint HINTS[] = {
#include "cardhints_data.h"           /* (a .h: the build remakes the ui files when one changes) */
};

/* the same options with a fix on (mods/fixes.c: "cardopts" unless named): as the cards were meant to be */
struct FixedHint { const char *fix; const char *card; int opt; const char *why; };
static const FixedHint FIXED[] = {
    { "university", "$UNIVE00", 0, "Not for about a week after a fruitless search here for the religious library." },
    { "university", "$UNIVE00", 1, "Not for about a week after a fruitless search here for alchemical formulae." },
    { "cardopts", "$SELEC01", 1, "Not for about three weeks after a sewer grate in this town defeated you." },
    { "cardopts", "$SELEC01", 2, "Needs the bribe in your purse; not for two days after a guard recognized you, nor for a day after guards spotted you sneaking at the wall." },
    { "cardopts", "$REVOL14", 4, "Needs a rope, and not after you have waited a day and come back to the walls." },
    { "cardopts", "$CITYW02", 0, "Needs a rope." },
    { "cardopts", "$CITYW04", 2, "" },                      /* (the climb: never greyed) */
    { "cardopts", "$CITYW04", 3, "" },
    { "cardopts", "$CITYW04", 4, "Not for about three weeks after a sewer grate here defeated you." },
    { "cardopts", "$CITYW04", 5, "" },
    { "cardopts", "$CITYW04", 8, "Not for two days after a guard refused your bribe here, nor if you cannot pay it." },
};
extern "C" int fix_is_on(const char *id);

extern "C" const char *card_hint(const char *card, int opt)
{
    if (!card || !*card) return nullptr;
    for (const FixedHint &h : FIXED)
        if (h.opt == opt && !strcasecmp(h.card, card) && fix_is_on(h.fix)) return *h.why ? h.why : nullptr;
    for (const CardHint &h : HINTS)
        if (h.opt == opt && !strcasecmp(h.card, card)) return h.why;
    return nullptr;
}
