// saints.cpp - the saints as Darklands+ tells them (the character sheet's Saints, the prayer screen,
// the tooltips): where a description's legend ends and its promise begins, the game's description
// with what the help really does (mods/saintinfo.h), whether encounters call on a saint. (What is at
// work on the party, the blessings and potions: ui/effects.cpp.)
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include "richtext.h"
#include "imgui.h"
ImU32 ns_tier_colour(int n);   /* ui/nsdraw.cpp */
#include "ui.h"
#include "gamedata.h"
#include "../mods/fixes.h"
#include "../mods/saintinfo.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../mods/gamemodel.h"
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
    ImGui::PushTextWrapPos(0); ui_rich_text(("What it does:\n" + saint_effects_text(s)).c_str()); ImGui::PopTextWrapPos();
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    if (*saint_info_note(s)) ImGui::TextWrapped("%s", saint_info_note(s));
    ImGui::PopStyleColor();
    const char *u = saint_info_unlike(s);
    if (*u) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.20f, 0.12f, 1)); ImGui::PushTextWrapPos(0); ui_rich_text(("Differs from the description: " + rich_auto(u)).c_str()); ImGui::PopTextWrapPos(); ImGui::PopStyleColor(); }
    {   int n = saint_info_cards(s);                       /* (coloured: many green, some blue, a few brown, none grey) */
        ImU32 c = ns_tier_colour(n >= 10 ? 20 : n >= 3 ? 5 : n);
        ImGui::PushStyleColor(ImGuiCol_Text, c); ImGui::TextWrapped("%s", saint_encounters_line(s)); ImGui::PopStyleColor(); }
}

/* whether encounters let a member call on the saint: its powers beyond its help */
const char *saint_encounters_line(int s)
{
    int n = saint_info_cards(s);
    return n >= 10 ? "Many encounters offer this saint as a choice, and there it can do more than the help above."
         : n >= 3 ? "Some encounters offer this saint as a choice, and there it can do more than the help above."
         : n >= 1 ? "A few encounters offer this saint as a choice, and there it can do more than the help above."
         : "No encounter calls for this saint: its help above is all it does.";
}
