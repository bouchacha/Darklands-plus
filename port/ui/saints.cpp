// saints.cpp - the saints (F12 > Saints): the blessings and potions in effect and how long they
// last, every saint's description from DARKLAND.SNT, who in the party knows them, and a finder by what they help with ("Strength", "fire", "heal",
// "travel on water" ...), like the saint finder of the Darklands Companion tool. Knowledge:
// saints_known, far 2e38:082c, 0x14 bytes per member, a bit per saint in DARKLAND.LST order,
// highest bit first.
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include "imgui.h"
#include "ui.h"
#include "gamedata.h"
#include "libraries.h"
#include "effects.h"
#include "../mods/fixes.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
}

static bool knows(int m, int s)
{
    return (rb(G_FAR2E38, (u16)(0x082c + m * 0x14 + (s >> 3))) & (0x80 >> (s & 7))) != 0;
}

static const char *ATTRS[7] = { "Endurance", "Strength", "Agility", "Perception", "Intelligence", "Charisma", "Divine Favor" };
static const char *SKILLS[19] = { "Edged Weapons", "Impact Weapons", "Flails", "Polearms", "Thrown Weapons", "Bows",
    "Missile Devices", "Alchemy", "Religion", "Virtue", "Speak Common", "Speak Latin", "Read & Write", "Healing",
    "Artifice", "Stealth", "Streetwise", "Riding", "Woodwise" };

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
std::vector<Eff> effects_now(int size)
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
        if (kind == 0x48 && k >= 0 && k < 7) snprintf(b, sizeof b, "%+d %s", t, ATTRS[k]);
        else if (kind == 0x49 && k >= 0 && k < 19) snprintf(b, sizeof b, "%+d %s", t, SKILLS[k]);
        else if (kind == 0x4a) {
            /* +0: 0x69 Deadly Blade; 0x6d Hardarmor (its grade), 0x16d a saint's armour, 0x16e St. Crispin's
               armour quality, 0x174 a saint's fire protection (armourday); 0x74 Firewall (armourday);
               0x6b Greatpower marked by greatpower; else an inventory index:
               Strongedge (3..5) or Greatpower (14/22/30) */
            if (k == 0x16d) { snprintf(b, sizeof b, "%+d armour (a saint's)", t); v.push_back({ m, b, left, kind }); continue; }
            if (k == 0x16e) { snprintf(b, sizeof b, "%+d armour quality (St. Crispin)", t); v.push_back({ m, b, left, kind }); continue; }
            if (k == 0x174) { snprintf(b, sizeof b, "fire protection %d (a saint's)", t); v.push_back({ m, b, left, kind }); continue; }
            int item = k == 0x69 || k == 0x6b || k == 0x6d || k == 0x74 ? k : t <= 5 ? 0x6a : 0x6b;
            std::string n = item < (int)gamedata.items.size() ? game_text(gamedata.items[item].name.c_str(), 64) : "Potion";
            if (item == 0x69) snprintf(b, sizeof b, "%s: %+d damage", n.c_str(), t);
            else if (item == 0x6a) snprintf(b, sizeof b, "%s: %+d penetration", n.c_str(), t);
            else if (item == 0x6d) { static const int T[4] = { 0, 1, 2, 2 }, Q[4] = { 0, 20, 25, 30 }; int g = t < 1 ? 1 : t > 3 ? 3 : t;
                snprintf(b, sizeof b, "%s: armour %+d thick, %+d quality", n.c_str(), T[g], Q[g]); }
            else if (item == 0x74) snprintf(b, sizeof b, "%s: fire protection %d", n.c_str(), t);
            else if (fix_is_on("greatpower")) snprintf(b, sizeof b, "%s: weapon quality %+d", n.c_str(), t);
            else if (!(t >> 4)) snprintf(b, sizeof b, "%s: %+d penetration", n.c_str(), t & 15);
            else snprintf(b, sizeof b, "%s: %+d damage, %+d penetration", n.c_str(), t >> 4, t & 15);
        } else continue;
        v.push_back({ m, b, left, kind });
    }
    return v;
}

static void effects(int size)
{
    std::vector<Eff> v = effects_now(size);
    v.erase(std::remove_if(v.begin(), v.end(), [](const Eff &e) { return e.kind == 0x43; }), v.end());   /* (blessings and potions only) */
    char head[64]; snprintf(head, sizeof head, "In effect now (%d)###effects", (int)v.size());
    if (!ImGui::CollapsingHeader(head, v.empty() ? 0 : ImGuiTreeNodeFlags_DefaultOpen)) return;
    if (v.empty()) { ImGui::TextDisabled("No blessings or potions are working on the party."); return; }
    std::sort(v.begin(), v.end(), [](const Eff &a, const Eff &b) { return a.m != b.m ? a.m < b.m : a.left < b.left; });
    if (ImGui::BeginTable("eff", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
        for (const Eff &e : v) {
            char n[16]; g_str(G_DGROUP, (u16)(G_MEMBER(e.m) + G_M_SHORTNAME), n, sizeof n);
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::TextUnformatted(game_text(n, sizeof n).c_str());
            ImGui::TableNextColumn(); ImGui::TextUnformatted(e.what.c_str());
            ImGui::TableNextColumn();
            if (e.left < 48) ImGui::TextDisabled("%ld hour%s left", e.left, e.left == 1 ? "" : "s");
            else ImGui::TextDisabled("%ld days left", (e.left + 12) / 24);
        }
        ImGui::EndTable();
    }
    ImGui::Spacing();
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

void ui_saint_text(const std::string &t)
{
    static std::string last; static size_t at;
    if (t != last) { last = t; at = saint_effect_at(t); }
    size_t a = at; while (a < t.size() && t[a] == ' ') a++;
    std::string lore = t.substr(0, at), eff = t.substr(a);
    while (!lore.empty() && lore.back() == ' ') lore.pop_back();
    while (!eff.empty() && eff.back() == ' ') eff.pop_back();
    if (!lore.empty()) ImGui::TextWrapped("%s", lore.c_str());
    if (!eff.empty()) {
        if (!lore.empty()) ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.10f, 0.25f, 0.52f, 1));
        ImGui::TextWrapped("%s", eff.c_str());
        ImGui::PopStyleColor();
    }
}

/* the effect part of a description as a line of its own: "He increases Charisma ..." ->
 * "Increases Charisma ..." */
static std::string saint_summary(const std::string &t)
{
    static std::map<const std::string *, std::string> cache;
    auto it = cache.find(&t);
    if (it != cache.end()) return it->second;
    size_t a = saint_effect_at(t);
    while (a < t.size() && t[a] == ' ') a++;
    std::string e = t.substr(a);
    for (const char *p : { "He ", "She ", "It ", "They " })
        if (e.compare(0, strlen(p), p) == 0) { e = e.substr(strlen(p)); if (!e.empty()) e[0] = (char)toupper((unsigned char)e[0]); break; }
    cache[&t] = e;
    return e;
}

extern "C" void tab_saints(void)
{
    if (gamedata.saints.empty()) { ImGui::TextWrapped("DARKLAND.LST could not be read."); return; }
    bool in_game = game_mode() != GM_OTHER;
    int size = in_game ? (s16)g_rw(G_PARTY_SIZE) : 0;
    if (size < 0 || size > 5) size = 0;
    static char find[64];
    static int known_only = 0;
    ImGui::TextWrapped("What each saint can do, and who in the party knows them. Find saints by what they help with. "
                       "A saint nobody knows shows the nearest library that teaches it (a town's monastery or university, or a monastery in the country: each teaches four saints, the same all game).");
    ImGui::SetNextItemWidth(UIS(300));
    ImGui::InputTextWithHint("##sfind", "Strength, fire, heal, travel, Virtue ...", find, sizeof find);
    static const char *QUICK[] = { "Endurance", "Strength", "Agility", "Perception", "Intelligence", "Charisma",
                                   "Edged", "Impact", "Flail", "Polearm", "Thrown", "Bow", "Missile", "Alchemy", "Religion",
                                   "Virtue", "Speak Latin", "Read/Write", "Healing", "Artifice", "Stealth", "Streetwise",
                                   "Riding", "Woodwise", "fire", "water", "heal", "reputation", "Divine Favor" };
    ImGui::SameLine();
    if (ImGui::BeginCombo("##quick", "Helps with...", ImGuiComboFlags_HeightLarge)) {
        for (const char *q : QUICK) if (ImGui::Selectable(q)) snprintf(find, sizeof find, "%s", q);
        ImGui::EndCombo();
    }
    if (in_game) {
        ImGui::RadioButton("All saints", &known_only, 0); ImGui::SameLine();
        ImGui::RadioButton("Known in the party", &known_only, 1); ImGui::SameLine();
        ImGui::RadioButton("Not yet known", &known_only, 2);
    }
    if (in_game && size > 0) effects(size);
    ImGui::Separator();
    std::string f = lower(find);
    int shown = 0;
    ImGui::BeginChild("saints", ImVec2(0, 0));
    for (int s = 0; s < (int)gamedata.saints.size(); s++) {
        const Saint &t = gamedata.saints[s];
        std::string who;
        int nk = 0;
        for (int m = 0; m < size; m++)
            if (knows(m, s)) {
                char n[16]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_SHORTNAME), n, sizeof n);
                if (!who.empty()) who += ", ";
                who += game_text(n, sizeof n); nk++;
            }
        if (known_only == 1 && !nk) continue;
        if (known_only == 2 && nk) continue;
        if (!f.empty() && lower(t.name).find(f) == std::string::npos && lower(t.text).find(f) == std::string::npos) continue;
        shown++;
        ImGui::PushID(s);
        bool open = ImGui::CollapsingHeader(t.name.c_str(), f.empty() ? 0 : ImGuiTreeNodeFlags_DefaultOpen);
        if (!open && ImGui::IsItemHovered() && ImGui::BeginTooltip()) {
            ImGui::PushTextWrapPos(UIS(520)); ui_saint_text(t.text); ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
        float full = ImGui::GetContentRegionAvail().x;
        /* where to learn it: the libraries that teach it, the nearest first (ui/libraries.h) */
        std::vector<LibraryHit> where;
        if (in_game && !nk) where = libraries_teaching(s);
        auto days = [](double d) { char b[24]; if (d < 0) b[0] = 0; else if (d < 0.6) snprintf(b, sizeof b, "here"); else snprintf(b, sizeof b, "%d day%s", (int)(d + 0.5), (int)(d + 0.5) == 1 ? "" : "s"); return std::string(b); };
        if (!open) {                       /* what the saint does, in a line, to scan the list by */
            float x0 = full * 0.30f, x1 = nk || !where.empty() ? full * 0.70f : full - UIS(10);
            std::string e = saint_summary(t.text), cut = e;
            while (!cut.empty() && ImGui::CalcTextSize((cut + "...").c_str()).x > x1 - x0) {
                size_t sp = cut.find_last_of(' '); cut = sp == std::string::npos ? std::string() : cut.substr(0, sp);
            }
            if (cut.size() < e.size()) cut += "...";
            ImGui::SameLine(x0); ImGui::TextDisabled("%s", cut.c_str());
        }
        if (nk) { ImGui::SameLine(full * (open ? 0.55f : 0.72f)); ImGui::TextDisabled("known by %s", who.c_str()); }
        else if (!where.empty() && !open) {
            ImGui::SameLine(full * 0.72f);
            std::string d = days(where[0].days);
            ImGui::TextDisabled("learn at %s%s%s", where[0].lib->name.c_str(), d.empty() ? "" : ", ", d.c_str());
        }
        if (open) {
            ImGui::Indent(UIS(20)); ui_saint_text(t.text);
            if (in_game && !nk) {
                std::string l;
                if (where.empty()) l = "No library teaches this saint in this game (a saint may still be granted by a card).";
                else {
                    l = "Taught in the library of ";
                    size_t n = where.size() < 6 ? where.size() : 6;
                    for (size_t i = 0; i < n; i++) {
                        std::string d = days(where[i].days);
                        l += (i ? (i + 1 == n && where.size() <= 6 ? " and " : ", ") : "") + where[i].lib->name + (d.empty() ? "" : " (" + d + ")");
                    }
                    if (where.size() > 6) l += " and " + std::to_string(where.size() - 6) + " more";
                    l += ".";
                }
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                ImGui::TextWrapped("%s", l.c_str());
                ImGui::PopStyleColor();
            }
            ImGui::Unindent(UIS(20)); ImGui::Spacing();
        }
        ImGui::PopID();
    }
    if (!shown) ImGui::TextDisabled("No saint matches.");
    ImGui::EndChild();
}
