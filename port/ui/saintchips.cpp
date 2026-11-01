// saintchips.cpp - a saint's help in parts (saintchips.h): the code's own account of it (mods/saintinfo.c
// saint_info_does) cut into chips for the Saints page (ui/sheetview.cpp draws them), each set against what the
// other saints give of the same thing; the account written out an effect a line (saint_effects_text, for the
// page's panel, the prayer screen, the margins and the tooltips); and what a help says of each attribute and
// skill, for the page's shields filter and its order. All of it follows the saints' fixes as they are now.
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <algorithm>
#include "saintchips.h"
#include "gamedata.h"
#include "richtext.h"
#include "effects.h"
extern "C" {
#include "../mods/gamemodel.h"
#include "../mods/saintinfo.h"
#include "../mods/fixes.h"
#include "../mods/sheetview.h"
}

static std::string lower(std::string s) { for (char &c : s) c = (char)tolower((unsigned char)c); return s; }

/* what a saint's help does (mods/saintinfo.c, the code's own account) names attribute or skill key
   (0..6 the attributes, 7..25 the skills, 26 the town's opinion): the shields' filter on the Saints page.
   The account says the skills shortly ("Edged", "Latin") and "every weapon skill" for the seven. */
static const char *const SAINT_SKILL_WORD[19] = { "Edged", "Impact", "Flails", "Polearms", "Thrown", "Bows", "Missile Devices", "Alchemy",
    "Religion", "Virtue", "Speak Common", "Latin", "Read & Write", "Healing", "Artifice", "Stealth", "Streetwise", "Riding", "Woodwise" };
bool saint_touches(int s, int key) { return saint_effect(s, key) > 0; }
int saint_key_count(int key)
{
    static int n[SK_KEYS], fixes = -1;
    int f = fix_is_on("sainttexts") | fix_is_on("armorday") << 1 | fix_is_on("fireward") << 2 | fix_is_on("pauls") << 3 | fix_is_on("saintrep") << 4;   /* (the account follows these fixes) */
    if (f != fixes) { fixes = f; for (int k = 0; k < SK_KEYS; k++) { n[k] = 0; for (int t = 0; t < (int)gamedata.saints.size() && t < SAINT_INFO_N; t++) n[k] += saint_touches(t, k); } }
    return key >= 0 && key < SK_KEYS ? n[key] : 0;
}

/* a saint's help in short, for its row on the Saints page: what saint_info_does says, a chip a part -
   a shield and how much it gives, from + to +++ against what the other saints give of it (- to --- what it
   costs), healing (a green cross before the shield), the town's opinion, the weapon skills (all of them:
   three shields; those of the weapons in hand: "in hand"), a few words for the rest; the most first, the
   part in full under the pointer, the whole account in the panel at the right */
static std::string span_of(const std::string &t)            /* "6 to 11" -> "6–11"; "half its maximum" -> "half" */
{
    std::string r = t;
    size_t p = r.find(" to "); if (p != std::string::npos) r.replace(p, 4, "\x01");   /* (a dash drawn: chip_text) */
    if (r == "half its maximum") r = "half";
    return r;
}
static int saint_key_of(const std::string &name)             /* an attribute (0..6) or a skill (7..25) by the account's word; -1 */
{
    for (int a = 0; a < 7; a++) if (name == gm_attr_name(a)) return a;
    for (int k = 0; k < 19; k++) if (name == SAINT_SKILL_WORD[k]) return 7 + k;
    return -1;
}
/* what is at work that a saint's sentence names (lower case), as roundels (ui/effects.h EI_*, as the margin's "Blessings and potions"
   and the potions' accounts show it): up to two (the weapon and the armor); how many */
static int saint_effect_icons(const std::string &low, int ic[2])
{
    int n = 0;
    if (low.find("does nothing") != std::string::npos) return 0;   /* (Fire protection with the fix "No invulnerability from fire protection") */
    if (low.find("travels") != std::string::npos) ic[n++] = low.find("woods") != std::string::npos ? EI_TRAVEL_WOODS : EI_TRAVEL;
    else if (low.find("fire protection") != std::string::npos) ic[n++] = EI_FIRE;
    else if (low.find("death from wounds") != std::string::npos) ic[n++] = EI_NO_DEATH;
    else if (low.find("quality") != std::string::npos) {
        if (low.find("weapon") != std::string::npos || low.find("blades") != std::string::npos) ic[n++] = EI_WEAPON_Q;
        if (low.find("armor") != std::string::npos) ic[n++] = EI_ARMOR_Q;
    } else if (low.find("armor +") != std::string::npos || (low.find("armor") != std::string::npos && low.find("thicker") != std::string::npos))
        ic[n++] = low.find("missiles") != std::string::npos ? EI_MISSILE_ARMOR : EI_ARMOR;
    return n;
}
/* the roundel's value, short as the margin has it: "x2" (travel), "50/90" (fire), "+10q" (quality); "" none */
static std::string saint_effect_value(int ei, const std::string &st)
{
    std::smatch m;
    if (ei == EI_TRAVEL || ei == EI_TRAVEL_WOODS) return st.find("twice") != std::string::npos ? "x2" : st.find("half again") != std::string::npos ? "x1.5" : "";
    if (ei == EI_FIRE) {
        std::string v; static const std::regex num("[0-9]+");
        for (std::sregex_iterator it(st.begin(), st.end(), num), end; it != end; ++it) v += (v.empty() ? "" : "/") + it->str();
        return v;
    }
    static const std::regex better("([0-9]+) points better"), plus("\\+([0-9]+)");
    if ((ei == EI_ARMOR_Q || ei == EI_WEAPON_Q) && std::regex_search(st, m, better)) return "+" + m[1].str() + "q";
    if ((ei == EI_ARMOR_Q || ei == EI_ARMOR || ei == EI_MISSILE_ARMOR) && std::regex_search(st, m, plus)) return "+" + m[1].str() + (ei == EI_ARMOR_Q ? "q" : "");
    if (ei == EI_ARMOR || ei == EI_MISSILE_ARMOR) return st.find("one level") != std::string::npos ? "+1" : st.find("two levels") != std::string::npos ? "+2" : "";
    return "";
}
static const struct { const char *has, *say; bool bad; } SAINT_WORDS[] = {
    { "does nothing", "does nothing", false }, { "cross water", "walks on water", false }, { "ambush", "no ambush", false },
    { "long sword", "a long sword", false }, { "Costs a point", "costs Strength, Virtue", true }, { "musical instrument", "with music: the town's opinion", false },
    { "nearest town", "carried to a town", false }, { "groschen", "a little money", false }, { "Teaches", "teaches a saint", false },
    { "Virtue is raised", "Virtue 20 at least, a quarter of the money", false }, { "No help of its own", "encounters only", false }, { "Fugger", "the bankers' favor", false },
    { "loses a tenth", "may cost money", true }, { "loses 3 to 8", "may cost Endurance", true },
    { "Costs", "costs something", true }
};
static void saint_chips_parse(const std::string &does, std::vector<SaintChip> &out)
{
    out.clear();
    /* the sentences, and in them the parts (", " and "; ") */
    std::vector<std::string> sent;
    for (size_t a = 0; a < does.size(); ) {
        size_t e = does.find(". ", a);
        while (e != std::string::npos && e >= 2 && does.compare(e - 2, 2, "St") == 0) e = does.find(". ", e + 2);   /* ("St. Cosmas": no end) */
        std::string t = does.substr(a, e == std::string::npos ? std::string::npos : e - a);
        while (!t.empty() && (t.back() == '.' || t.back() == ' ')) t.pop_back();
        if (!t.empty()) sent.push_back(t);
        if (e == std::string::npos) break;
        a = e + 2;
    }
    static const std::regex heal("(all|half the|[0-9]+%|[0-9]+ to [0-9]+)( of)?( lost)? (Endurance|Strength)( and (Endurance|Strength))?");
    static const std::regex amount("^(.*?)\\s*(\\+[0-9]+( to [0-9]+)?|lowered by ([0-9]+ to [0-9]+|half its maximum))(.*)$");
    static const std::regex rep("by ([0-9]+ to [0-9]+)");
    for (const std::string &st : sent) {
        std::string low = lower(st);
        bool used = false;
        if (low.find("heals") != std::string::npos || low.find("healed") != std::string::npos) {
            for (std::sregex_iterator it(st.begin(), st.end(), heal), end; it != end; ++it) {
                std::string v = (*it)[1].str(); if (v == "half the") v = "half";
                SaintChip c{ CH_HEAL, (*it)[4].str() == "Endurance" ? ATTR_END : ATTR_STR, span_of(v), false, st };
                out.push_back(c);
                if ((*it)[6].matched) { c.key = (*it)[6].str() == "Endurance" ? ATTR_END : ATTR_STR; out.push_back(c); }
                used = true;
            }
        }
        static const std::regex floor_("(Endurance|Strength)( and (Endurance|Strength))? below [0-9]+ (is|are) raised to ([0-9]+)");
        { std::smatch m; if (std::regex_search(st, m, floor_)) {                /* healed up to a floor: "to 9" */
            out.push_back({ CH_HEAL, m[1].str() == "Endurance" ? ATTR_END : ATTR_STR, "to " + m[5].str(), false, st });
            if (m[3].matched) out.push_back({ CH_HEAL, m[3].str() == "Endurance" ? ATTR_END : ATTR_STR, "to " + m[5].str(), false, st });
            used = true; } }
        static const std::regex cost("^Costs ([0-9]+ to [0-9]+) (Endurance|Strength)");
        { std::smatch m; if (std::regex_search(st, m, cost)) { out.push_back({ CH_RAISE, m[2].str() == "Endurance" ? ATTR_END : ATTR_STR, "-" + span_of(m[1].str()), true, st }); used = true; } }
        if (low.find("reputation") != std::string::npos) {
            std::smatch m; std::string v = std::regex_search(st, m, rep) ? span_of(m[1].str()) : "";
            bool down = low.find("lower") != std::string::npos;
            out.push_back({ CH_REP, KEY_REP, (down ? "-" : "+") + v, down, st });
            used = true;
        }
        /* the rises and falls: names (", " and " and ") before "+a to b" or "lowered by" */
        std::vector<std::string> pending; std::string pend_part;
        size_t a = 0;
        while (a <= st.size()) {
            size_t e = st.find_first_of(",;", a);
            std::string part = st.substr(a, e == std::string::npos ? std::string::npos : e - a);
            while (!part.empty() && part[0] == ' ') part.erase(0, 1);
            if (part.rfind("and ", 0) == 0) part.erase(0, 4);
            if (part.rfind("Every member's ", 0) == 0) part.erase(0, 15);
            std::smatch m;
            std::string lp = lower(part);
            if (std::regex_match(part, m, amount) && lp.find("heals") == std::string::npos && lp.find("healed") == std::string::npos) {   /* (the skill Healing is a name) */
                std::string names = m[1].str(), v = m[2].str();
                if (names.size() > 5 && names.compare(names.size() - 5, 5, " each") == 0) names.erase(names.size() - 5);   /* ("Streetwise and Woodwise each +1 to 4") */
                bool down = v.rfind("lowered", 0) == 0;
                std::string val = down ? "-" + span_of(m[4].str()) : "+" + span_of(v.substr(1));
                std::string pl = pend_part + (pend_part.empty() ? "" : ", ") + part;
                /* the names: "A and B", "every weapon skill", "the skills of the weapons in hand"; the armor (one chip
                   whatever its words: "worn body and leg armor", "it gives the one prayed for armor") */
                std::vector<std::string> ns = pending;
                if (lower(names).find("armor") != std::string::npos) {   /* (its roundel: armor, or its quality) */
                    SaintChip c{ CH_WORDS, -1, "armor " + val, down, pl };
                    c.icon = m[5].str().rfind(" quality", 0) == 0 ? EI_ARMOR_Q : EI_ARMOR;
                    c.val = val + (c.icon == EI_ARMOR_Q ? "q" : "");
                    out.push_back(c); names.clear();
                }
                for (size_t b = 0; b < names.size(); ) {
                    size_t f = names.find(" and ", b);
                    std::string n = names.substr(b, f == std::string::npos ? std::string::npos : f - b);
                    if (n.rfind("A woman: ", 0) == 0 || n.rfind("a woman: ", 0) == 0) n = n.substr(9);
                    if (n.rfind("in a town, ", 0) == 0) n = n.substr(11);
                    if (!n.empty()) ns.push_back(n);
                    if (f == std::string::npos) break;
                    b = f + 5;
                }
                for (const std::string &n : ns) {
                    std::string ln = lower(n);
                    if (ln.find("weapons in hand") != std::string::npos) out.push_back({ CH_INHAND, -1, val, down, pl });
                    else if (ln.find("weapon skill") != std::string::npos) out.push_back({ CH_WEAPONS, -1, val, down, pl });
                    else if (ln.find("skill") != std::string::npos || ln.find("attribute") != std::string::npos) out.push_back({ CH_WORDS, -1, n + " " + val, down, pl });
                    else { int k = saint_key_of(n); if (k >= 0) out.push_back({ CH_RAISE, k, val, down, pl }); else out.push_back({ CH_WORDS, -1, n + " " + val, down, pl, true }); }
                }
                pending.clear(); pend_part.clear(); used = true;
            } else if (saint_key_of(part) >= 0) { pending.push_back(part); pend_part += (pend_part.empty() ? "" : ", ") + part; }
            if (e == std::string::npos) break;
            a = e + 1;
        }
        int ic[2], nic = used ? 0 : saint_effect_icons(low, ic);
        for (int i = 0; i < nic; i++) {                        /* what is at work: its roundel */
            SaintChip c{ CH_WORDS, -1, saint_effect_value(ic[i], st), false, st };
            c.icon = ic[i]; out.push_back(c); used = true;
        }
        if (!used) {                                           /* the rest: a few words */
            const char *say = nullptr; bool bad = false;
            for (const auto &w : SAINT_WORDS) if (low.find(lower(w.has)) != std::string::npos) { say = w.say; bad = w.bad; break; }
            out.push_back({ CH_WORDS, -1, say ? say : st.substr(0, 18) + "...", bad, st, !say });
        }
    }
}
/* what a saint's help does (saint_info_does), for the panel at the right, the prayer screen and the tooltips: each
   effect on its line, the attributes and skills as their shields; names sharing one value side by side (a bracket
   under them) with the value once, lines of the same value joined ("{Charisma} +6 to 11" and "{Riding} and {Speak
   Common} +6 to 11": "{Charisma}{Riding}{Speak Common} +6 to 11"); the conditions in words. In the order of the
   saint's chips (the most first, the costs, the words last: saint_chips_update), as the list has them */
std::string saint_effects_text(int s)
{
    std::string does = saint_info_does(s);
    std::vector<std::string> lines, fx;                         /* (fx: a sentence's roundels, before its first line) */
    for (size_t a = 0; a < does.size(); ) {                     /* the sentences ("St. Cosmas": no end) */
        size_t e = does.find(". ", a);
        while (e != std::string::npos && e >= 2 && does.compare(e - 2, 2, "St") == 0) e = does.find(". ", e + 2);
        std::string st = does.substr(a, e == std::string::npos ? std::string::npos : e - a);
        while (!st.empty() && (st.back() == '.' || st.back() == ' ')) st.pop_back();
        /* the clauses (", " and "; "), names alone held for the clause that gives their value; what is at work its roundel first
           (the chips', the margin's) */
        std::string pend, round;
        { int ic[2], nic = saint_effect_icons(lower(st), ic); for (int i = 0; i < nic; i++) round += rich_effect(ic[i]) + " "; }
        size_t first = lines.size();
        for (size_t b = 0; b <= st.size(); ) {
            size_t f = st.find_first_of(",;", b);
            std::string part = st.substr(b, f == std::string::npos ? std::string::npos : f - b);
            while (!part.empty() && part[0] == ' ') part.erase(0, 1);
            std::string bare = part.rfind("and ", 0) == 0 ? part.substr(4) : part;
            if (saint_key_of(bare) >= 0 && f != std::string::npos) pend += (pend.empty() ? "" : ", ") + part;
            else if (!part.empty()) { lines.push_back((pend.empty() ? "" : pend + ", ") + part); pend.clear(); }
            if (f == std::string::npos) break;
            b = f + 1;
        }
        if (!pend.empty()) lines.push_back(pend);
        fx.resize(lines.size());
        if (lines.size() > first) fx[first] = round;
        if (e == std::string::npos) break;
        a = e + 2;
    }
    /* each line's place: the first chip whose words hold it or are held by it; a line no chip has, after the one before */
    const std::vector<SaintChip> &chips = saint_chips(s);
    std::vector<int> rank(lines.size(), -1);
    for (size_t i = 0; i < lines.size(); i++) {
        for (size_t k = 0; k < chips.size() && rank[i] < 0; k++) {
            const std::string &p = chips[k].part;
            if (!p.empty() && (p.find(lines[i]) != std::string::npos || lines[i].find(p) != std::string::npos)) rank[i] = (int)k;
        }
        if (rank[i] < 0) rank[i] = i ? rank[i - 1] : -1;
    }
    static const std::regex sep("\\}(, and |, | and )\\{"), pure("^((?:\\{[^{}:]+\\})+) (\\+[0-9]+(?: to [0-9]+)?)$");
    std::vector<std::string> out; std::vector<std::string> val; std::vector<int> orank;
    for (size_t li = 0; li < lines.size(); li++) {
        std::string &l = lines[li];
        std::string r = std::regex_replace(rich_auto(l.c_str()), sep, "}{");
        if (!r.empty()) r[0] = (char)toupper((unsigned char)r[0]);
        r = fx[li] + r;
        std::smatch m;
        if (std::regex_match(r, m, pure)) {                       /* a value given before: these names join that line */
            bool joined = false;
            for (size_t i = 0; i < out.size(); i++) if (val[i] == m[2].str()) { out[i] = out[i].substr(0, out[i].size() - val[i].size() - 1) + m[1].str() + " " + val[i]; orank[i] = std::min(orank[i], rank[li]); joined = true; break; }
            if (joined) continue;
            out.push_back(r); val.push_back(m[2].str()); orank.push_back(rank[li]);
        } else { out.push_back(r); val.push_back(""); orank.push_back(rank[li]); }
    }
    std::vector<size_t> ord(out.size());
    for (size_t i = 0; i < ord.size(); i++) ord[i] = i;
    std::stable_sort(ord.begin(), ord.end(), [&](size_t a, size_t b) { return orank[a] < orank[b]; });
    std::string t;
    for (size_t i : ord) t += (t.empty() ? "" : "\n") + out[i];
    return t;
}
/* how much a chip gives, to set one saint's against another's: a rise its middle ("+6 to 11": 8.5), healing
   as a part of what is lost (all 100, half 50, 30%: 30, points three times their middle, a floor "to 9" 20),
   the town's opinion its middle (10 without a number); a cost or a fall below 0; words 0 */
static float chip_mag(const SaintChip &c)
{
    if (c.kind == CH_WORDS) return 0;
    std::string v = c.val; float sign = 1;
    if (!v.empty() && (v[0] == '-' || v[0] == '+')) { if (v[0] == '-') sign = -1; v.erase(0, 1); }
    float m;
    if (v.empty()) m = 10;
    else if (v == "all") m = 100;
    else if (v == "half") m = 50;
    else if (v.rfind("to ", 0) == 0) m = 20;
    else if (v.back() == '%') m = (float)atoi(v.c_str());
    else {
        size_t d = v.find('\x01');
        int a = atoi(v.c_str()), b = d == std::string::npos ? a : atoi(v.c_str() + d + 1);
        m = (a + b) * 0.5f * (c.kind == CH_HEAL ? 3 : 1);
    }
    return sign * m;
}
/* which chips a chip is set against: the same attribute or skill (healing apart; all the weapon skills
   together), or where fewer than 4 saints give it, its kind (the attributes, the weapon skills, the other
   skills, healing); the costs together */
static int chip_pool(const SaintChip &c, bool group)
{
    if (c.mag < 0) return 900;
    switch (c.kind) {
    case CH_HEAL: return group ? 303 : 100 + c.key;
    case CH_REP: return KEY_REP;
    case CH_WEAPONS: case CH_INHAND: return group ? 301 : 200;
    case CH_RAISE: return !group ? (c.key >= 7 && c.key < 14 ? 200 : c.key) : c.key < 7 ? 300 : c.key < 14 ? 301 : 302;
    }
    return -1;
}
/* every saint's chips, set against one another (again when a fix changes the account) */
static std::vector<std::vector<SaintChip>> chip_sets;
static void saint_chips_update(void)
{
    static int last = -1;
    int sig = fix_is_on("sainttexts") | fix_is_on("armorday") << 1 | fix_is_on("fireward") << 2 | fix_is_on("pauls") << 3 | fix_is_on("saintrep") << 4 | (sheetview_shared() ? 32 : 0);
    if (sig == last && (int)chip_sets.size() == SAINT_INFO_N) return;
    last = sig;
    chip_sets.assign(SAINT_INFO_N, std::vector<SaintChip>());
    std::map<int, std::vector<float>> pool;
    for (int t = 0; t < SAINT_INFO_N; t++) {
        saint_chips_parse(saint_info_does(t), chip_sets[t]);
        for (SaintChip &c : chip_sets[t]) {
            c.mag = chip_mag(c);
            if (c.kind == CH_WORDS) continue;
            pool[chip_pool(c, false)].push_back(fabsf(c.mag));
            pool[chip_pool(c, true)].push_back(fabsf(c.mag));
        }
    }
    for (std::vector<SaintChip> &v : chip_sets) {
        for (SaintChip &c : v) {
            if (c.kind == CH_WORDS) continue;
            const std::vector<float> *pl = &pool[chip_pool(c, false)];
            if (pl->size() < 4) pl = &pool[chip_pool(c, true)];
            float less = 0, same = 0, m = fabsf(c.mag);
            for (float x : *pl) { if (x < m) less++; else if (x == m) same++; }
            float at = (less + 0.5f * same) / (float)pl->size();
            c.tier = at < 1.0f / 3 ? 1 : at < 2.0f / 3 ? 2 : 3;
        }
        std::stable_sort(v.begin(), v.end(), [](const SaintChip &a, const SaintChip &b) {   /* the most first, then the costs, the words last */
            int ga = a.kind == CH_WORDS && a.icon < 0 ? 2 : a.mag < 0, gb = b.kind == CH_WORDS && b.icon < 0 ? 2 : b.mag < 0;
            if (ga != gb) return ga < gb;
            if (a.tier != b.tier) return a.tier > b.tier;
            return fabsf(a.mag) > fabsf(b.mag);
        });
    }
}
const std::vector<SaintChip> &saint_chips(int s)
{
    static const std::vector<SaintChip> none;
    saint_chips_update();
    return s >= 0 && s < (int)chip_sets.size() ? chip_sets[s] : none;
}
/* how much saint s gives of attribute or skill key (KEY_REP: the town's opinion), to put the saints in order: the
   best of its chips that raise or heal it (the weapon skills: every one, and those in hand) - its +, then a rise
   before healing, then how much; 0 none */
float saint_effect(int s, int key)
{
    float best = 0;
    for (const SaintChip &c : saint_chips(s)) {
        if (c.mag <= 0) continue;
        bool hit = c.kind == CH_REP ? key == KEY_REP : c.kind == CH_WEAPONS || c.kind == CH_INHAND ? key >= 7 && key < 14 : (c.kind == CH_RAISE || c.kind == CH_HEAL) && c.key == key;
        float score = c.tier * 1000.0f + (c.kind == CH_HEAL ? 0 : 500) + fminf(c.mag, 499);
        if (hit && score > best) best = score;
    }
    return best;
}
/* test (usaintchips): every saint's help through the parse, with the saints' fixes on and then off; the parts
   it did not know are logged, with a count at the end */
extern "C" void saintchips_check(void)
{
    static const char *IDS[] = { "sainttexts", "armorday", "fireward", "pauls", "saintrep" };
    int idx[5], was[5], n = 0, raw = 0;
    for (int f = 0; f < 5; f++) { idx[f] = -1; for (int i = 0; i < FIX_COUNT; i++) if (!strcmp(FIXES[i].id, IDS[f])) idx[f] = i; was[f] = idx[f] >= 0 ? FIXES[idx[f]].on : 0; }
    for (int pass = 1; pass >= 0; pass--) {
        for (int f = 0; f < 5; f++) if (idx[f] >= 0) FIXES[idx[f]].on = pass;   /* (the texts only: no hook, no setting touched) */
        for (int t = 0; t < SAINT_INFO_N; t++) {
            std::vector<SaintChip> ch; saint_chips_parse(saint_info_does(t), ch); n++;
            for (const SaintChip &c : ch) if (c.raw) { raw++; fprintf(stderr, "saintchips: %s (fixes %s) not understood: %s\n", gamedata.saints[t].name.c_str(), pass ? "on" : "off", c.part.c_str()); }
        }
    }
    for (int f = 0; f < 5; f++) if (idx[f] >= 0) FIXES[idx[f]].on = was[f];
    fprintf(stderr, "saintchips: %d helps, %d parts not understood\n", n, raw);
}
