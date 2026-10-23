// lodgeview.cpp - an inn's room or a camp drawn by Darklands+ (mods/lodgeview.c runs it): the
// members with each one's day, what the chosen member can do (the original's words; greyed with
// the reason where not), the town's teachers with their fees, what a day costs against the purse,
// and Spend a day, Until rested and Leave.
#include <stdio.h>
#include <string>
#include "nsdraw.h"
void ui_draw_hourglass(ImDrawList *dl, ImVec2 tl, float kx, float k, int hour, int day, int month);   /* ui/hourglass.cpp */
#include "gamepics.h"
const GamePic *ns_figure_pic(int cache, const char *code, const unsigned char *mp, const char *suffix);
extern "C" {
#include "../runtime/cpu.h"
#include "../mods/lodgeview.h"
#include "../mods/gamemodel.h"
#include "money.h"
extern "C" {
#include "../mods/teachers.h"
#include "../mods/fixes.h"
}
}


static std::string money(long pf) { return money_str(pf); }

extern int cardview_dialog, game_dialog_kind;
extern "C" int lodgeview_showing(void) { return lodge_model.open && ns_screen == NS_LODGE && !ns_hidden() && !(cardview_dialog && !game_dialog_kind); }
extern "C" int lodgeview_camping(void) { return lodge_model.mode == 1; }

extern "C" void lodgeview_draw(void)
{
    const LodgeModel &m = lodge_model;
    if (!lodgeview_showing()) return;
    NsDraw P;
    if (!P.begin()) return;
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM;
    const ImU32 PANEL = IM_COL32(244, 234, 210, 255), HOVER = IM_COL32(232, 214, 176, 255), SEL = IM_COL32(222, 196, 146, 255);
    const ImU32 BROWN = IM_COL32(120, 84, 40, 235), GREY = IM_COL32(150, 136, 112, 140), GREEN = IM_COL32(40, 92, 34, 235), RED = IM_COL32(112, 40, 22, 235);
    char b[160];
    if (m.passing) P.click = false;                         /* (the hours going by: a click skips them, mods/lodgeview.c) */
    P.page();
    std::string place = ns_gtext(m.place);
    std::string title = m.mode == 1 ? "Camping" : m.mode == 0 ? "A room at the inn" : "Staying here";
    if (m.mode != 1 && !place.empty()) title += ", " + place;
    P.text(f, NsDraw::FS_TITLE, 65, 2.5f, ink, title);
    if (m.mode == 1) snprintf(b, sizeof b, "You live off the land, but the lord may be upset if you are caught: after %d day%s the risk is %d.", m.days, m.days == 1 ? "" : "s", m.risk);
    else snprintf(b, sizeof b, "A day here costs %s for the room.%s", money(m.lodging).c_str(), m.days ? (" Days stayed: " + std::to_string(m.days) + ".").c_str() : "");
    P.para(fo, NsDraw::FS_SMALL, 66, 13, 220, soft, b);

    /* the members and their days */
    const float LX0 = 63, LX1 = 186, TY = 23, BY = 170;
    P.rect(LX0, TY, LX1, BY, PANEL, 1.8f);
    int n = 0; for (int i = 0; i < 5; i++) n += m.m[i].present;
    /* a row each: the name and the day chosen; strength and divine favor only where they are
       short (the party panel beside shows them all; they decide Regain Strength and Pray) */
    float rh = n ? fminf(21.0f, (BY - TY - 2) / n) : 21;
    float y = TY + 1;
    for (int i = 0; i < 5; i++) {
        const LodgeMember &p = m.m[i];
        if (!p.present) continue;
        bool sel = m.sel == i, hov = P.over(LX0 + 1, y, LX1 - 1, y + rh - 1);
        if (sel) { P.rect(LX0 + 1, y, LX1 - 1, y + rh - 1, SEL, 1.2f); P.rect(LX0 + 1, y, LX0 + 2.6f, y + rh - 1, IM_COL32(140, 70, 20, 230)); }
        else if (hov) P.rect(LX0 + 1, y, LX1 - 1, y + rh - 1, HOVER, 1.2f);
        if (P.hit(LX0 + 1, y, LX1 - 1, y + rh - 1)) ns_push(LV_SELECT, i, 0, 0);
        float nx = LX0 + 6;
        std::string shortv;
        if (p.str_max > 0 && p.str < p.str_max) shortv = "Str " + std::to_string(p.str) + "/" + std::to_string(p.str_max);
        if (p.df_max > 0 && p.df < p.df_max) shortv += (shortv.empty() ? "" : "   ") + std::string("DF ") + std::to_string(p.df) + "/" + std::to_string(p.df_max);
        float sw = shortv.empty() ? 0 : P.width(fo, NsDraw::FS_SMALL, shortv) + 4;
        P.text(f, NsDraw::FS_NAME, nx, y + 2.0f, ink, ns_gtext(p.name), -1, LX1 - 3 - sw);
        if (!shortv.empty()) P.text(fo, NsDraw::FS_SMALL, 0, y + 2.8f, soft, shortv, LX1 - 3);
        std::string line = ns_gtext(p.line);
        if (p.act == 6 && p.skill >= 0 && p.skill < 19) {
            int fee = 0; for (int t = 0; t < m.nteach; t++) if (m.t[t].skill == p.skill) fee = m.t[t].fee;
            snprintf(b, sizeof b, "Train or Study: %s (%s a day)", gm_skill_name(p.skill), money_str(fee).c_str());
            line = b;
        }
        P.text(fo, NsDraw::FS_BODY, nx, y + 10.5f, p.act ? IM_COL32(92, 40, 12, 255) : soft, line, -1, LX1 - 3);
        y += rh;
    }

    /* the chosen member's choices */
    const float RX0 = 189, RX1 = 317;
    P.rect(RX0, TY, RX1, BY, PANEL, 1.8f);
    if (m.sel >= 0 && m.sel < 5 && m.m[m.sel].present) {
        const LodgeMember &p = m.m[m.sel];
        snprintf(b, sizeof b, "%s's day", ns_gtext(p.name).c_str());
        P.text(f, NsDraw::FS_HEAD, RX0 + 3, TY + 2, ink, b);
        static const char *ACT[7] = { "Just Relax", "Regain Strength", "Pray for divine favor", "Alchemy work", "Earn a little money", "Guard the camp", "Train or study" };
        static const char *WHY[7] = { "", "Already at full Strength.", "Divine Favor is full.", "Knows no formula to make.", "Only at an inn in a town.", "Only when camping in the wild.", "No teacher in this place now." };
        static const char *WHAT[7] = { "Rest: Endurance comes back.", "Rest and care: Strength comes back; the better the party's Healing, the more.",
            "Prayer: Divine Favor comes back; the better Religion and Virtue, the more.", "Make potions from a known formula (the ingredients are used up).",
            "Work in the town for a wage that depends on the member's skills.", "Keep watch and find food: the camp is safer.", "Lessons from a teacher of the town, for a fee each day." };
        float ay = TY + 12, ah = 9.6f;
        int hover_act = -1;
        for (int a = 0; a < 7; a++) {
            bool can = p.can[a], cur = p.act == a;
            float x0 = RX0 + 3, x1 = RX1 - 3;
            bool h = P.over(x0, ay, x1, ay + ah - 1);
            if (h) hover_act = a;
            ImU32 bg = cur ? IM_COL32(140, 70, 20, 235) : !can ? IM_COL32(222, 210, 186, 255) : h ? IM_COL32(214, 190, 146, 255) : IM_COL32(230, 214, 180, 255);
            P.rect(x0, ay, x1, ay + ah - 1, bg, 1.2f);
            P.text(f, NsDraw::FS_NAME * 0.92f, x0 + 3, ay + 1.6f, cur ? NsDraw::LIGHT : can ? ink : dim, ACT[a]);
            if (cur && a != 6) P.text(fo, 3.8f, 0, ay + 2.6f, NsDraw::LIGHT, "chosen", x1 - 2);
            if (can && a != 6 && P.hit(x0, ay, x1, ay + ah - 1) && !cur) ns_push(LV_ACT, a, m.sel, 0);
            ay += ah;
            if (a == 6 && can) {                                    /* the teachers: one each */
                for (int t = 0; t < m.nteach; t++) {
                    const auto &T = m.t[t];
                    bool capped = teach_capped(m.sel, T.skill, T.val) != 0;     /* (learnt all this one teaches) */
                    bool tc = p.act == 6 && p.skill == T.skill, th = P.over(x0 + 6, ay, x1, ay + 8);
                    P.rect(x0 + 6, ay, x1, ay + 8, tc ? IM_COL32(140, 70, 20, 235) : capped ? IM_COL32(222, 210, 186, 255) : th ? IM_COL32(214, 190, 146, 255) : IM_COL32(236, 224, 196, 255), 1.0f);
                    P.text(fo, NsDraw::FS_BODY, x0 + 9, ay + 1.2f, tc ? NsDraw::LIGHT : capped ? dim : ink, gm_skill_name(T.skill));
                    if (capped) snprintf(b, sizeof b, "learnt all (%d)", T.val);
                    else snprintf(b, sizeof b, "%s a day", money_str(T.fee).c_str());
                    P.text(fo, NsDraw::FS_SMALL, 0, ay + 1.8f, tc ? NsDraw::LIGHT : capped ? dim : soft, b, x1 - 2);
                    if (th) hover_act = 100 + t;
                    if (P.hit(x0 + 6, ay, x1, ay + 8) && !tc && !capped) ns_push(LV_TEACH, t, m.sel, 0);
                    ay += 8.6f;
                }
            }
        }
        /* about the one under the pointer */
        std::string about;
        if (hover_act >= 0 && hover_act < 7) about = std::string(WHAT[hover_act]) + (p.can[hover_act] ? "" : std::string(" ") + WHY[hover_act]);
        if (hover_act == 3 && m.mode != 1) about += " At an inn a failed batch can blow up (to see how often, point at a formula's risk on the alchemy screen); in a camp in the wild it cannot.";
        else if (hover_act >= 100) {
            const auto &T = m.t[hover_act - 100];
            const char *sk = gm_skill_name(T.skill);
            int now = T.skill >= 0 && T.skill < 19 ? gm_member_skill(m.sel, T.skill) : 0;
            int c = teach_chance(T.val), days = c > 0 ? (100 + c / 2) / c : 0;
            if (teach_capped(m.sel, T.skill, T.val))
                snprintf(b, sizeof b, "Teacher of %s, own skill %d. %s's %s is %d already: nothing more to learn from this teacher.",
                         sk, T.val, ns_gtext(p.name).c_str(), sk, now);
            else
                snprintf(b, sizeof b, "Teacher of %s, own skill %d. Fee: %s a day. Each day of lessons: %s %d%% chance of +1 (about one point every %d day%s)%s. %s's %s: %d.",
                         sk, T.val, money_with_total(T.fee).c_str(), gm_a_an(c), c, days, days == 1 ? "" : "s", fix_is_on("teachcap") ? ", up to the teacher's own skill" : "",
                         ns_gtext(p.name).c_str(), sk, now);
            about = b;
        } else if (m.nteach == 0 && m.mode != 1) about = "No teacher offers lessons in this town now. Teachers are found by asking the masters of a guild, the university, the monks, the physician or the alchemists (a lectern marks the option on their cards; the margin lists what the town offers); they then teach here for a week.";
        if (!about.empty()) P.para(fo, NsDraw::FS_SMALL, RX0 + 4, fmaxf(ay + 2, BY - 22), RX1 - RX0 - 8, soft, about);
    }

    /* the day's cost, the purse; the buttons */
    long after = m.purse - m.cost;
    b[0] = 0;
    if (m.cost > m.purse) snprintf(b, sizeof b, "A day costs %s in all: more than the purse holds.", money(m.cost).c_str());
    else if (m.cost) snprintf(b, sizeof b, "A day %s %s in all; the purse %s %s.", m.cost < 0 ? "earns" : "costs", money(m.cost < 0 ? -m.cost : m.cost).c_str(), m.cost < 0 ? "will hold" : "is left with", money(after).c_str());
    if (b[0]) P.para(fo, NsDraw::FS_BODY, 64, 172, 133, m.cost > m.purse ? NsDraw::RED : ink, b);
    /* a day the purse cannot pay: the game lets no time pass, yet counts it as a day stayed; so no
       day is offered (members who earn a little money may pay for it) */
    bool pay = m.cost <= m.purse;
    float bx = 199;
    if (P.button(bx, 174, bx + 22, 184, "A day", pay ? GREEN : GREY) && pay) ns_push(LV_DAY, 0, 0, 0);
    if (P.button(bx + 24, 174, bx + 48, 184, "A week", pay ? GREEN : GREY) && pay) ns_push(LV_DAYS, 7, 0, 0);
    bool hurt = false;                                      /* (someone short of strength who only relaxes: Until rested has them regain it) */
    for (int i = 0; i < 5; i++) if (m.m[i].present && m.m[i].act == 0 && m.m[i].str < m.m[i].str_max && m.m[i].can[1]) hurt = true;
    bool rest = (m.resting || hurt) && pay;
    if (P.button(bx + 50, 174, bx + 84, 184, "Until rested", rest ? BROWN : GREY) && rest) ns_push(LV_REST, 0, 0, 0);
    if (!pay && P.over(bx, 174, bx + 84, 184))
        P.para(fo, 3.9f, 190, 186, 126, soft, "The purse cannot pay for a day. Members set to Earn a little money may make up the difference; otherwise, leave.");
    else if (P.over(bx + 50, 174, bx + 84, 184))
        P.para(fo, 3.9f, 190, 186, 126, soft, m.resting || hurt ? (hurt ? "Those short of Strength who only relax regain it; then days pass until no one is regaining Strength or praying (at most 14; it stops if anything happens, or when a day cannot be paid)."
                                                                        : "Days pass until no one is regaining Strength or praying any more (at most 14; it stops if anything happens, or when a day cannot be paid).")
                                                     : "Nobody is short of Strength, regaining it or praying.");
    else if (P.over(bx + 24, 174, bx + 48, 184))
        P.para(fo, 3.9f, 190, 186, 126, soft, "Seven days, each member doing what was chosen (it stops if anything happens, or when a day cannot be paid).");
    else P.text(fo, 3.9f, 190, 188, dim, "F11: list of keys");
    if (P.button(bx + 86, 174, bx + 117, 184, "Leave", RED, NsDraw::FS_SMALL, PB_Y)) ns_push(LV_LEAVE, 0, 0, 0);

    /* the hours going by: the game's own clock (ui/hourglass.cpp), with the day of a run of days
       above it and how to skip below */
    if (m.passing) {
        P.rect(60, 0, 320, 200, IM_COL32(20, 12, 4, 120));
        const float CX0 = 130, CY0 = 44;
        std::string head = m.pass_n == 0 ? (m.mode == 1 ? "A day in camp" : "A day at the inn")
                         : m.pass_of ? "Day " + std::to_string(m.pass_n) + " of " + std::to_string(m.pass_of)
                                     : "Day " + std::to_string(m.pass_n) + ", until rested";
        float hw = P.width(f, NsDraw::FS_HEAD, head);
        P.rect(190 - hw * 0.5f - 5, CY0 - 13, 190 + hw * 0.5f + 5, CY0 - 3, IM_COL32(40, 26, 12, 230), 2);
        P.text(f, NsDraw::FS_HEAD, 190 - hw * 0.5f, CY0 - 12, IM_COL32(244, 226, 180, 255), head);
        ui_draw_hourglass(P.dl, NsDraw::w(CX0, CY0), P.kx, P.k, m.pass_hour, m.pass_day, m.pass_month);
        const char *hint = m.pass_n ? "Click or press a key to skip to the end" : "Click or press a key to skip";
        float tw = P.width(fo, NsDraw::FS_SMALL, hint);
        P.rect(190 - tw * 0.5f - 4, CY0 + 121, 190 + tw * 0.5f + 4, CY0 + 129, IM_COL32(40, 26, 12, 220), 2);
        P.text(fo, NsDraw::FS_SMALL, 190 - tw * 0.5f, CY0 + 122, IM_COL32(230, 214, 178, 255), hint);
    }
}
