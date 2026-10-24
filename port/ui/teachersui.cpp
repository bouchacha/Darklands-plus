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
            snprintf(b, sizeof b, "Lessons go up to %d, the teacher's own ability, and no further (a Darklands+ fix; in the original game students could reach 99).\n", s.strength);
        else
            snprintf(b, sizeof b, "The teacher's own ability is %d, but lessons go past it: students can reach 99, as in the original game.\n", s.strength);
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

/* a lectern: a big book lying open on it (its cover showing under the pages, the gutter shaded, a
   ribbon hanging), on a short slanted desk, a turned column with two rings, a broad foot. ink: the
   wood, the cover and the lines; page: the paper */
/* (the same ink everywhere on light ground, IM_COL32(80, 50, 24): the cards, Go to, the inn, the margin) */
void ui_draw_teach_mark(ImDrawList *dl, ImVec2 c, float s, unsigned ink, unsigned page)
{
    auto P = [&](float x, float y) { return ImVec2(c.x + x * s, c.y + y * s); };
    float th = s * 0.06f > 1.0f ? s * 0.06f : 1.0f;
    /* the cover, a little larger than the pages, under them */
    ImVec2 cov[6] = { P(-0.56f, -0.38f), P(0.0f, -0.29f), P(0.56f, -0.38f), P(0.56f, 0.03f), P(0.0f, 0.10f), P(-0.56f, 0.03f) };
    { ImVec2 l[4] = { cov[0], cov[1], cov[4], cov[5] }, r[4] = { cov[1], cov[2], cov[3], cov[4] };
      dl->AddConvexPolyFilled(l, 4, ink); dl->AddConvexPolyFilled(r, 4, ink); }
    /* the pages: each bowed up from the gutter */
    for (int side = -1; side <= 1; side += 2) {
        float sx = (float)side;
        dl->PathClear();
        dl->PathLineTo(P(0.0f, -0.27f));
        dl->PathBezierCubicCurveTo(P(sx * 0.16f, -0.38f), P(sx * 0.36f, -0.40f), P(sx * 0.51f, -0.35f), 10);
        dl->PathLineTo(P(sx * 0.51f, -0.02f));
        dl->PathBezierCubicCurveTo(P(sx * 0.36f, -0.07f), P(sx * 0.16f, -0.05f), P(0.0f, 0.05f), 10);
        dl->PathFillConcave(page);
        for (int k = 0; k < 4; k++) {                                  /* lines of writing, following the page's bow */
            float y = -0.27f + k * 0.065f;
            dl->AddBezierQuadratic(P(sx * 0.09f, y + 0.02f), P(sx * 0.24f, y - 0.05f), P(sx * 0.43f, y - 0.03f), ink, th * 0.5f, 6);
        }
    }
    dl->AddLine(P(0.0f, -0.27f), P(0.0f, 0.05f), ink, th * 1.2f);      /* the gutter */
    dl->AddLine(P(0.03f, 0.05f), P(0.07f, 0.20f), IM_COL32(176, 40, 30, 255), th * 1.3f);   /* the ribbon */
    /* the desk under the book */
    ImVec2 desk[4] = { P(-0.40f, 0.08f), P(0.40f, 0.08f), P(0.30f, 0.16f), P(-0.30f, 0.16f) };
    dl->AddConvexPolyFilled(desk, 4, ink);
    /* the column, two rings, the foot */
    ImVec2 col[4] = { P(-0.055f, 0.16f), P(0.055f, 0.16f), P(0.045f, 0.40f), P(-0.045f, 0.40f) };
    dl->AddConvexPolyFilled(col, 4, ink);
    dl->AddRectFilled(P(-0.10f, 0.22f), P(0.10f, 0.25f), ink, s * 0.01f);
    dl->AddRectFilled(P(-0.10f, 0.35f), P(0.10f, 0.38f), ink, s * 0.01f);
    ImVec2 foot[4] = { P(-0.10f, 0.40f), P(0.10f, 0.40f), P(0.26f, 0.48f), P(-0.26f, 0.48f) };
    dl->AddConvexPolyFilled(foot, 4, ink);
}
