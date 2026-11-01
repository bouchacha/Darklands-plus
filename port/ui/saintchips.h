// saintchips.h - a saint's help in parts (saintchips.cpp), from the code's own account of it (mods/saintinfo.h,
// which follows the saints' fixes): the chips of the Saints page, set against what the other saints give, and what
// they say of an attribute or a skill (the page's shields filter and its order). The account written out a line an
// effect, saint_effects_text, is declared in gamedata.h (the prayer screen, the margins and the tooltips use it).
#pragma once
#include <string>
#include <vector>

/* a part of the help: a rise (or a cost) of an attribute or skill, healing, the town's opinion, the weapon skills (all
   of them, or those of the weapons in hand), or words - a roundel (EI_*, ui/effects.h) where they name what is at work */
enum { CH_RAISE, CH_HEAL, CH_REP, CH_WEAPONS, CH_INHAND, CH_WORDS };
struct SaintChip {
    int kind, key; std::string val; bool bad; std::string part;
    bool raw = false;                                         /* words the parse did not know (usaintchips) */
    float mag = 0; int tier = 0;                              /* how much (chip_mag); 1..3 against the other saints */
    int icon = -1;                                            /* words for what is at work, as its roundel (EI_*: the margin's, the potions'), val short */
};
enum { SK_KEYS = 27, KEY_REP = 26 };                          /* the keys a help can raise: attributes 0..6, skills 7..25, the town's opinion */
const std::vector<SaintChip> &saint_chips(int s);             /* the most first (against the other saints), the costs, the words last */
float saint_effect(int s, int key);                           /* how much of key it gives, to put the saints in order (0 none) */
bool saint_touches(int s, int key);                           /* it raises or heals key */
int saint_key_count(int key);                                 /* how many saints' help does */
extern "C" void saintchips_check(void);                       /* test (usaintchips): every help through the parse, the parts it did not know logged */
