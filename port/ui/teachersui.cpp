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
    static const char *const N[] = { "Blacksmiths", "Swordsmiths", "Armourers", "Bowyers", "Artificers", "The guild",
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
            snprintf(b, sizeof b, "Lessons go up to %d, no further.\n", s.strength);
        else
            snprintf(b, sizeof b, "Lessons are given at strength %d (in the original game there is no limit: students can reach 99).\n", s.strength);
        t += b;
        snprintf(b, sizeof b, "Each day of lessons: %s %d%% chance of +1 to the skill (about one point every %d day%s).\n", gm_a_an(c), c, days, days == 1 ? "" : "s");
        t += b;
    }
    t += "\nHow to hire: " + std::string(s.ask) + " The teacher then stays in town for a week; at the inn, set a member to \"Train or study\".";
    if (s.cond && *s.cond) t += "\nWho they accept: " + std::string(s.cond);
    if (s.now[0]) t += "\nRight now: " + std::string(s.now);
    return t;
}

std::string ui_teachers_hired_line(int loc)
{
    TchHired h[16];
    int n = teach_hired(loc, h, 16);
    std::string r;
    for (int k = 0; k < n; k++) {
        char b[80];
        if (h[k].skill < 0 || h[k].skill >= 19) continue;
        snprintf(b, sizeof b, "%s (until %d %s)", skill_short(h[k].skill), h[k].day, gm_month_name((h[k].month % 12 + 12) % 12));
        r += (r.empty() ? "" : ", ") + std::string(b);
    }
    return r;
}

/* a lectern: a book lying open on its slanted desk (the pages bowed up from the spine, a ribbon
   hanging), the desk on a turned post with a knop, three feet. ink: the wood and the lines; page */
void ui_draw_teach_mark(ImDrawList *dl, ImVec2 c, float s, unsigned ink, unsigned page)
{
    auto P = [&](float x, float y) { return ImVec2(c.x + x * s, c.y + y * s); };
    float th = s * 0.07f > 1.0f ? s * 0.07f : 1.0f;
    /* the desk: a slanted board, seen a little from above */
    ImVec2 desk[4] = { P(-0.46f, -0.04f), P(0.46f, -0.04f), P(0.40f, 0.07f), P(-0.40f, 0.07f) };
    dl->AddConvexPolyFilled(desk, 4, ink);
    /* the book: each page a curve up from the spine to its outer edge */
    for (int side = -1; side <= 1; side += 2) {
        float sx = (float)side;
        dl->PathClear();
        dl->PathLineTo(P(0.0f, -0.22f));                                    /* the spine, top */
        dl->PathBezierCubicCurveTo(P(sx * 0.14f, -0.34f), P(sx * 0.34f, -0.36f), P(sx * 0.50f, -0.30f), 8);   /* the top edge */
        dl->PathLineTo(P(sx * 0.48f, 0.00f));                               /* the outer edge */
        dl->PathBezierCubicCurveTo(P(sx * 0.32f, -0.06f), P(sx * 0.12f, -0.04f), P(0.0f, 0.04f), 8);          /* the bottom edge */
        dl->PathFillConcave(page);
        dl->PathClear();
        dl->PathLineTo(P(0.0f, -0.22f));
        dl->PathBezierCubicCurveTo(P(sx * 0.14f, -0.34f), P(sx * 0.34f, -0.36f), P(sx * 0.50f, -0.30f), 8);
        dl->PathLineTo(P(sx * 0.48f, 0.00f));
        dl->PathBezierCubicCurveTo(P(sx * 0.32f, -0.06f), P(sx * 0.12f, -0.04f), P(0.0f, 0.04f), 8);
        dl->PathStroke(ink, ImDrawFlags_Closed, th);
        for (int k = 0; k < 3; k++) {                                       /* lines of writing */
            float y = -0.20f + k * 0.065f;
            dl->AddLine(P(sx * 0.10f, y), P(sx * 0.40f, y - 0.03f), ink, th * 0.55f);
        }
    }
    dl->AddLine(P(0.0f, -0.22f), P(0.0f, 0.04f), ink, th);                 /* the spine */
    dl->AddLine(P(0.02f, 0.04f), P(0.06f, 0.16f), IM_COL32(170, 40, 30, 255), th * 1.2f);   /* the ribbon */
    /* the post: turned, a knop halfway */
    ImVec2 post[4] = { P(-0.045f, 0.07f), P(0.045f, 0.07f), P(0.035f, 0.40f), P(-0.035f, 0.40f) };
    dl->AddConvexPolyFilled(post, 4, ink);
    dl->AddEllipseFilled(P(0.0f, 0.22f), ImVec2(0.08f * s, 0.04f * s), ink);
    /* the feet */
    dl->AddLine(P(0.0f, 0.39f), P(-0.24f, 0.47f), ink, th * 1.3f);
    dl->AddLine(P(0.0f, 0.39f), P(0.24f, 0.47f), ink, th * 1.3f);
    dl->AddLine(P(0.0f, 0.39f), P(0.0f, 0.47f), ink, th * 1.3f);
}
