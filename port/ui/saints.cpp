// saints.cpp - the saints as Darklands+ tells them (the character sheet's Saints, the prayer screen,
// the tooltips): where a description's legend ends and its promise begins, the game's description
// with what the help really does (mods/saintinfo.h), whether encounters call on a saint; and the
// blessings and potions at work now and how long they last (effects_now: the margin's "In force").
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include "imgui.h"
#include "ui.h"
#include "gamedata.h"
#include "effects.h"
#include "../mods/fixes.h"
#include "../mods/saintinfo.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../mods/gamemodel.h"
}


/* hours since year 0 (the game's calendar: 365 days, no leap years; month from 0, day from 1) */
static long abs_hours(int y, int mo, int d, int h)
{
    static const int DAYS[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    long days = (long)y * 365;
    for (int i = 0; i < mo && i < 12; i++) days += DAYS[i];
    return (days + d) * 24 + h;
}

/* the running temporary effects: events of kind 0x48 (attribute: a saint's boost, or the
   lowering some saints bring with it), 0x49 (skill), 0x4a (weapon potion); +0 what, +0x1a member,
   +0x1e amount, +0x12 end (hour, day, month, year) */
static std::vector<Eff> effects_table(int size)
{
    std::vector<Eff> v;
    long now = abs_hours((s16)g_rw(G_CUR_YEAR), (s16)g_rw(G_CUR_MONTH), (s16)g_rw(G_CUR_DAY), (s16)g_rw(G_CUR_HOUR));
    for (int i = 0; i < G_NEVENTS; i++) {
        u32 p = g_event(i);
        if (!p) continue;
        u16 sg = (u16)(p >> 16), o = (u16)p;
        int kind = (s16)rw(sg, (u16)(o + 0x22));
        if (kind == 0x43) {                                 /* a companion's stay (add_member: type0 the slot; its end, the day they leave) */
            int m = (s16)rw(sg, o);
            long left = abs_hours((s16)rw(sg, (u16)(o + 0x18)), (s16)rw(sg, (u16)(o + 0x16)), (s16)rw(sg, (u16)(o + 0x14)), (s16)rw(sg, (u16)(o + 0x12))) - now;
            if (m >= 0 && m < size && left > 0 && left < 24L * 365 * 50) v.push_back({ m, "stays with the party", left, kind });
            continue;
        }
        if (kind < 0x48 || kind > 0x4a) continue;
        int m = (s16)rw(sg, (u16)(o + 0x1a)), k = (s16)rw(sg, o), t = (s16)rw(sg, (u16)(o + 0x1e));
        if (m < 0 || m >= size || t == 0 || (t < 0 && kind == 0x4a)) continue;     /* (a saint's lowering of an attribute: negative) */
        long left = abs_hours((s16)rw(sg, (u16)(o + 0x18)), (s16)rw(sg, (u16)(o + 0x16)), (s16)rw(sg, (u16)(o + 0x14)), (s16)rw(sg, (u16)(o + 0x12))) - now;
        if (left <= 0) continue;
        char b[96];
        if ((kind == 0x48 && k >= 0 && k < 7) || (kind == 0x49 && k >= 0 && k < 19)) {
            snprintf(b, sizeof b, "%+d %s", t, kind == 0x48 ? gm_attr_name(k) : gm_skill_name(k));
            Eff e = { m, b, left, kind }; e.k = k; e.amount = t;
            v.push_back(e);
            continue;
        }
        if (kind == 0x4a) {
            /* +0: 0x69 Deadly Blade; 0x6d Hardarmor (its grade), 0x16d a saint's armour, 0x16e a saint's
               armour quality, 0x174 a saint's fire protection (armourday); 0x74 Firewall (armourday);
               0x6b Greatpower marked by greatpower; else an inventory index:
               Strongedge (3..5) or Greatpower (14/22/30) */
            /* (a saint's, mods/sainttexts.c: 0x16f-0x173 armour and weapon quality, 0x175 armour against
               missiles, 0x177 / 0x178 the party's travel faster) */
            static const struct { int k; const char *f; } SAINTLY[] = {
                { 0x16d, "%+d armour (from a saint)" }, { 0x16e, "worn armour %+d quality (from a saint)" },
                { 0x16f, "metal armour %+d quality (from a saint)" }, { 0x170, "leather and cloth armour %+d quality (from a saint)" },
                { 0x171, "weapon %+d quality (from a saint)" }, { 0x172, "edged weapon %+d quality (from a saint)" },
                { 0x173, "blades, maces, polearms, crossbows and guns %+d quality (from a saint)" },
                { 0x174, "fire protection %d (from a saint)" }, { 0x175, "%+d armour against missiles (from a saint)" },
                { 0x177, "the party travels %d%% faster (from a saint)" }, { 0x178, "the party travels %d%% faster in woods (from a saint)" },
                { 0x179, "no death from wounds (from a saint)" } };
            bool done = false;
            if ((k == 0x177 || k == 0x178) && (t == 50 || t == 100)) {   /* (the speed in words) */
                snprintf(b, sizeof b, "the party travels %s%s (from a saint)", t == 100 ? "twice as fast" : "half again as fast", k == 0x178 ? " in woods" : "");
                Eff e = { m, b, left, kind }; e.k = k; e.amount = t; v.push_back(e); continue;
            }
            for (const auto &x : SAINTLY) if (k == x.k) { snprintf(b, sizeof b, x.f, t); Eff e = { m, b, left, kind }; e.k = k; e.amount = t; v.push_back(e); done = true; break; }
            if (done) continue;
            int item = k == 0x69 || k == 0x6b || k == 0x6d || k == 0x74 ? k : t <= 5 ? 0x6a : 0x6b;
            std::string n = item < (int)gamedata.items.size() ? game_text(gamedata.items[item].name.c_str(), 64) : "Potion";
            if (item == 0x69) snprintf(b, sizeof b, "%s: %+d damage", n.c_str(), t);
            else if (item == 0x6a) snprintf(b, sizeof b, "%s: %+d penetration", n.c_str(), t);
            else if (item == 0x6d) { static const int T[4] = { 0, 1, 2, 2 }, Q[4] = { 0, 20, 25, 30 }; int g = t < 1 ? 1 : t > 3 ? 3 : t;
                snprintf(b, sizeof b, "%s: armour %+d thickness, %+d quality", n.c_str(), T[g], Q[g]); }
            else if (item == 0x74) snprintf(b, sizeof b, "%s: fire protection %d", n.c_str(), t);
            else if (fix_is_on("greatpower")) snprintf(b, sizeof b, "%s: weapon quality %+d", n.c_str(), t);
            else if (!(t >> 4)) snprintf(b, sizeof b, "%s: %+d penetration", n.c_str(), t & 15);
            else snprintf(b, sizeof b, "%s: %+d damage, %+d penetration", n.c_str(), t >> 4, t & 15);
        } else continue;
        v.push_back({ m, b, left, kind });
    }
    /* of the protections that do not add up (fire, the party's speed), only the strongest of each member's */
    std::vector<Eff> out;
    for (size_t i = 0; i < v.size(); i++) {
        const Eff &e = v[i];
        bool weaker = false;
        if (e.kind == 0x4a && (e.k == 0x174 || e.k == 0x177 || e.k == 0x178))
            for (size_t j = 0; j < v.size() && !weaker; j++) {
                const Eff &o = v[j];
                if (j != i && o.m == e.m && o.kind == 0x4a && o.k == e.k &&
                    (o.amount > e.amount || (o.amount == e.amount && (o.left > e.left || (o.left == e.left && j < i))))) weaker = true;
            }
        if (!weaker) out.push_back(e);
    }
    return out;
}
/* what is at work now. While a battle has closed the cards the event table holds only what the
   battle made (mods/fixes.c): what was at work when it began is kept from then and shown with it
   (its hours as they were; a battle takes minutes) */
std::vector<Eff> effects_now(int size)
{
    static std::vector<Eff> before; static int before_seed = -1;
    int seed = (u16)g_rw(G_PARTY_SEED);
    std::vector<Eff> v = effects_table(size);
    if (!fixes_cards_down()) { before = v; before_seed = seed; return v; }
    if (seed != before_seed) return v;                      /* (a game loaded in a battle: nothing from before) */
    for (const Eff &e : before) if (e.m < size) v.push_back(e);
    return v;
}

static std::string lower(std::string s) { std::transform(s.begin(), s.end(), s.begin(), ::tolower); return s; }

/* A saint's description is the legend first, then what the saint does: "He increases Charisma
 * ...", "Provides a small increase to ...", "Invoking Lazarus can ...", "..., Dunstan enhances
 * ...". Found by the sentence that starts with a doer and an effect verb (or the bare verb);
 * failing that, the first sentence that speaks of increasing, enhancing, healing or restoring. */
static bool word_in(const std::string &w, const char *const *list)
{
    for (; *list; list++) if (w == *list) return true;
    return false;
}

size_t saint_effect_at(const std::string &t)
{
    static const char *VERB[] = { "increases", "improves", "provides", "grants", "gives", "protects", "heals", "cures", "allows",
        "reduces", "helps", "makes", "aids", "adds", "raises", "lowers", "prevents", "restores", "bestows", "enhances", "boosts",
        "can", "will", "may", "offers", "confers", "causes", 0 };
    static const char *ADV[] = { "also", "greatly", "modestly", "slightly", "sometimes", "often", "then", "only", "both", 0 };
    static const char *LEAD[] = { "invoking", "invoked", "prayer", "prayers", 0 };
    static const char *SUBJ[] = { "he", "she", "it", "they", 0 };
    auto words = [&](size_t at, std::vector<std::string> &w) {
        w.clear();
        size_t i = at;
        while (w.size() < 8 && i < t.size()) {
            while (i < t.size() && (t[i] == ' ' || t[i] == '"')) i++;
            size_t j = i; while (j < t.size() && (isalpha((unsigned char)t[j]) || t[j] == '\'')) j++;
            if (j == i) break;
            w.push_back(t.substr(i, j - i));
            i = j;
            if (i < t.size() && t[i] != ' ') break;               /* the clause ends at punctuation */
        }
    };
    auto effect = [&](const std::vector<std::string> &w, bool sentence) {
        if (w.empty()) return false;
        std::string w0 = lower(w[0]);
        bool cap = isupper((unsigned char)w[0][0]);
        size_t k = 0;
        if (sentence) {
            if (word_in(w0, LEAD)) return true;
            if (w0 == "if" && w.size() > 1 && lower(w[1]) == "invoked") return true;
            if (w0 == "when" && w.size() > 3 && lower(w[2]) == "is" && lower(w[3]) == "invoked") return true;
            size_t b = 0; while (b < w.size() && word_in(lower(w[b]), ADV)) b++;   /* "Provides ...", "Also heals ..." */
            if (b < w.size() && word_in(lower(w[b]), VERB) && b < w.size() && (b > 0 || cap)) return true;
        }
        if (!(word_in(w0, SUBJ) || (cap && w0 != "the" && w0 != "patron" && w0 != "saint"))) return false;
        k = 1;
        while (k < w.size() && word_in(lower(w[k]), ADV)) k++;
        return k < w.size() && word_in(lower(w[k]), VERB);
    };
    std::vector<std::string> w;
    size_t sent = 0;                                              /* where the current sentence began */
    for (size_t i = 0; i <= t.size(); i++) {
        bool start = i == 0;
        if (i > 0 && i < t.size() && t[i - 1] == ' ') {
            size_t p = i - 1; while (p > 0 && t[p] == ' ') p--;
            if (t[p] == '"' && p > 0) p--;
            if (strchr(".;:!?", t[p])) start = true;
            else if (t[p] == ',') {                               /* "..., Agnes grants": the whole sentence is about it */
                words(i, w);
                if (effect(w, false)) return sent;
                continue;
            }
        }
        if (!start) continue;
        if (i > 0) { size_t p = i - 1; while (p > 0 && t[p] == ' ') p--; if (t[p] != ';' && t[p] != ':') sent = i; }
        words(i, w);
        if (effect(w, true)) return i;
    }
    /* failing that, the first sentence that speaks of the effect */
    static const char *FB[] = { "increase", "increases", "enhance", "enhances", "enhanced", "heal", "heals", "restore", "restores", "improve", "improves", 0 };
    size_t s0 = 0;
    for (size_t i = 0; i <= t.size(); i++) {
        if (i == t.size() || ((t[i] == '.' || t[i] == '!' || t[i] == '?') && (i + 1 >= t.size() || t[i + 1] == ' ' || t[i + 1] == '"'))) {
            std::string s = lower(t.substr(s0, i - s0));
            for (size_t a = 0; a < s.size(); ) {
                while (a < s.size() && !isalpha((unsigned char)s[a])) a++;
                size_t b = a; while (b < s.size() && isalpha((unsigned char)s[b])) b++;
                if (b > a && word_in(s.substr(a, b - a), FB)) return s0;
                a = b;
            }
            s0 = i + 1; while (s0 < t.size() && (t[s0] == ' ' || t[s0] == '"')) s0++;
        }
    }
    return t.size();
}

/* a saint, as Darklands+ tells it: the game's description, then what the help really does (the
   game's code: mods/saintinfo.h), where the description promises otherwise, and whether encounters
   call for the saint (its other powers) */
void ui_saint_info(int s)
{
    if (s < 0 || s >= (int)gamedata.saints.size()) return;
    ImGui::TextWrapped("%s", gamedata.saints[s].text.c_str());
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.10f, 0.25f, 0.52f, 1));
    ImGui::TextWrapped("What it does: %s", saint_info_does(s));
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    if (*saint_info_note(s)) ImGui::TextWrapped("%s", saint_info_note(s));
    ImGui::PopStyleColor();
    const char *u = saint_info_unlike(s);
    if (*u) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.20f, 0.12f, 1)); ImGui::TextWrapped("Differs from the description: %s", u); ImGui::PopStyleColor(); }
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::TextWrapped("%s", saint_encounters_line(s));
    ImGui::PopStyleColor();
}

/* whether encounters let a member call on the saint: its powers beyond its help */
const char *saint_encounters_line(int s)
{
    int n = saint_info_cards(s);
    return n >= 10 ? "Many encounters let you call on this saint (its other powers work there)."
         : n >= 3 ? "Some encounters let you call on this saint (its other powers work there)."
         : n >= 1 ? "A few encounters let you call on this saint (its other powers work there)."
         : "No encounter calls for this saint: its help above is all it does.";
}
