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

extern "C" const char *card_hint(const char *card, int opt)
{
    if (!card || !*card) return nullptr;
    for (const CardHint &h : HINTS)
        if (h.opt == opt && !strcasecmp(h.card, card)) return h.why;
    return nullptr;
}
