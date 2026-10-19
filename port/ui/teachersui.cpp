// teachersui.cpp - the teachers a town can offer, as shown (ui/teachersui.h).
#include "imgui.h"
#include <stdio.h>
#include <string>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "gamedata.h"
#include "money.h"
#include "townicons.h"
#include "teachersui.h"
extern "C" {
#include "../mods/gamemodel.h"
}
#include "../mods/fixes.h"

bool ui_teachers_on(void) { return config_get_int("assist.teachers", 1) && !config_get_int("original", 0); }

TchTown ui_town_facts(int loc)
{
    TchTown t = {};
    if (loc < 0 || loc >= (int)gamedata.cities.size()) return t;
    const City &c = gamedata.cities[loc];
    t.size = c.size; t.buildings = c.buildings; t.seed = c.seed;
    for (int k = 0; k < 9; k++) t.shopq[k] = c.shopq[k];
    t.physician = citynav_town_offers(loc, TI_PHYSICIAN);
    t.alchemist = citynav_town_offers(loc, TI_ALCHEMIST);
    return t;
}

int ui_town_teachers(int loc, TchSource *out, int max)
{
    if (loc < 0 || loc >= (int)gamedata.cities.size()) return 0;
    TchTown t = ui_town_facts(loc);
    return teach_town(loc, &t, out, max);
}

const char *ui_teacher_short(int src)
{
    static const char *const N[] = { "Blacksmiths", "Swordsmiths", "Armorers", "Bowyers", "Artificers", "The guild",
                                     "University", "Monastery", "Physician", "Alchemists" };
    return src >= 0 && src < (int)(sizeof N / sizeof N[0]) ? N[src] : "?";
}

static const char *skill_short(int k)
{
    static const char *const S[19] = { "Edged", "Impact", "Flails", "Polearms", "Thrown", "Bows", "Missile Devices", "Alchemy",
        "Religion", "Virtue", "Speak Common", "Latin", "Read & Write", "Healing", "Artifice", "Stealth", "Streetwise", "Riding", "Woodwise" };
    return k >= 0 && k < 19 ? S[k] : "?";
}

std::string ui_teacher_skills(const TchSource &s)
{
    std::string r;
    for (int k = 0; k < s.n; k++) r += (k ? ", " : "") + std::string(skill_short(s.skill[k]));
    return r;
}

/* what a teacher's tooltip says: what is taught, how well and for how much, how to hire one, who
   is accepted and how it stands now - each a line of its own, the label first */
std::string ui_teacher_text(const TchSource &s)
{
    std::string t = std::string(s.who) + "\n";
    if (!s.n) t += "Teaches: nothing that can be learned.\n";
    else {
        bool same = true;
        for (int k = 1; k < s.n; k++) if (s.fee[k] != s.fee[0]) same = false;
        t += "Teaches: ";
        for (int k = 0; k < s.n; k++) {
            t += (k == 0 ? "" : k + 1 == s.n ? " and " : ", ") + std::string(gm_skill_name(s.skill[k]));
            if (!same) t += " (" + money_str(s.fee[k]) + " a day)";
        }
        t += ".\n";
        if (same) t += "Fee: " + money_str(s.fee[0]) + " per student per day.\n";
        int c = teach_chance(s.strength), days = c > 0 ? (100 + c / 2) / c : 0;
        char b[240];
        if (fix_is_on("teachcap"))
            snprintf(b, sizeof b, "The teacher's own skill: %d. Students can be taught up to %d, no further.\n", s.strength, s.strength);
        else
            snprintf(b, sizeof b, "The teacher's own skill: %d (in the original game it sets no limit: up to 99).\n", s.strength);
        t += b;
        snprintf(b, sizeof b, "Each day of lessons: %s %d%% chance of +1 to the skill (about one point every %d days).\n", gm_a_an(c), c, days);
        t += b;
    }
    t += "\nHow to hire: " + std::string(s.ask) + " The teacher then stays in town for a week; at the inn, set a member to \"Train or study\".";
    if (s.cond && *s.cond) t += "\nWho they accept: " + std::string(s.cond);
    if (s.now[0]) t += "\nRight now: " + std::string(s.now);
    return t;
}

std::string ui_teachers_hired_line(int loc)
{
    static const char *const MON[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
    TchHired h[16];
    int n = teach_hired(loc, h, 16);
    std::string r;
    for (int k = 0; k < n; k++) {
        char b[80];
        if (h[k].skill < 0 || h[k].skill >= 19) continue;
        snprintf(b, sizeof b, "%s (until %d %s)", skill_short(h[k].skill), h[k].day, MON[(h[k].month % 12 + 12) % 12]);
        r += (r.empty() ? "" : ", ") + std::string(b);
    }
    return r;
}

/* a lectern: an open book (its pages light, a line written on each) on a desk, a post, a foot */
void ui_draw_teach_mark(ImDrawList *dl, ImVec2 c, float s, unsigned ink, unsigned page)
{
    auto P = [&](float x, float y) { return ImVec2(c.x + x * s, c.y + y * s); };
    float th = s * 0.075f > 1.0f ? s * 0.075f : 1.0f;
    ImVec2 L[4] = { P(-0.52f, -0.38f), P(-0.03f, -0.25f), P(-0.03f, 0.03f), P(-0.52f, -0.10f) };
    ImVec2 R[4] = { P(0.03f, -0.25f), P(0.52f, -0.38f), P(0.52f, -0.10f), P(0.03f, 0.03f) };
    dl->AddConvexPolyFilled(L, 4, page); dl->AddConvexPolyFilled(R, 4, page);
    dl->AddPolyline(L, 4, ink, ImDrawFlags_Closed, th); dl->AddPolyline(R, 4, ink, ImDrawFlags_Closed, th);
    for (int k = 0; k < 2; k++) {
        float y = -0.22f + k * 0.12f;
        dl->AddLine(P(-0.42f, y - 0.03f), P(-0.13f, y + 0.05f), ink, th * 0.6f);
        dl->AddLine(P(0.13f, y + 0.05f), P(0.42f, y - 0.03f), ink, th * 0.6f);
    }
    dl->AddRectFilled(P(-0.36f, 0.03f), P(0.36f, 0.11f), ink);                     /* the desk */
    dl->AddRectFilled(P(-0.05f, 0.11f), P(0.05f, 0.40f), ink);                     /* the post */
    dl->AddRectFilled(P(-0.25f, 0.38f), P(0.25f, 0.46f), ink, s * 0.03f);          /* the foot */
}
