// professions.h - the professions of character creation: which the game offers (its own test,
// s037 18c8:22be, re-done here so a profession not offered can say why), what each one needs in
// words, and what it leads to.
#pragma once
#include <string>
#include <vector>

struct CgCtx {
    int age = 0, child = -1, female = 0, last = -1, prev = -1;   // last: the profession just done, prev: the one before
    signed char attr[6] = {}, skill[19] = {};
    unsigned char taken[37] = {};
};

bool cg_prof_rule(int p, const CgCtx &c);                 // the game's test for profession p alone
std::vector<int> cg_prof_offered(const CgCtx &c);          // what the game lists (it stops at 15)
const char *cg_prof_needs(int p);                          // what profession p asks, in words
std::vector<int> cg_prof_leads_to(int p);                  // the professions p helps open
std::string cg_prof_short(int p, const CgCtx &c);          // the attributes and skills this character lacks for p ("" none, or the past decides)
std::string cg_prof_why(int p, const CgCtx &c);            // why p is not open, in a sentence ("" open)
