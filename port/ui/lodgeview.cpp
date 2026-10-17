// lodgeview.cpp - an inn's room or a camp drawn by Darklands+ (mods/lodgeview.c runs it): the
// members with each one's day, what the chosen member can do (the original's words; greyed with
// the reason where not), the town's teachers with their fees, what a day costs against the purse,
// and Spend a day, Until rested and Leave.
#include <stdio.h>
#include <string>
#include "nsdraw.h"
#include "gamepics.h"
const GamePic *ns_figure_pic(int cache, const char *code, const unsigned char *mp, const char *suffix);
extern "C" {
#include "../runtime/cpu.h"
#include "../mods/lodgeview.h"
#include "../mods/gamemodel.h"
#include "money.h"
}

static const char *SKILL[19] = { "Edged Weapons", "Impact Weapons", "Flails", "Polearms", "Thrown Weapons", "Bows",
    "Missile Devices", "Alchemy", "Religion", "Virtue", "Speak Common", "Speak Latin", "Read & Write", "Healing",
    "Artifice", "Stealth", "Streetwise", "Riding", "Woodwise" };

static std::string money(long pf) { return money_str(pf); }

extern "C" void lodgeview_draw(void)
{
    const LodgeModel &m = lodge_model;
    extern int cardview_dialog;
    if (!m.open || ns_screen != NS_LODGE || ns_hidden() || cardview_dialog) return;
    NsDraw P;
    if (!P.begin()) return;
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM;
    const ImU32 PANEL = IM_COL32(244, 234, 210, 255), HOVER = IM_COL32(232, 214, 176, 255), SEL = IM_COL32(222, 196, 146, 255);
    const ImU32 BROWN = IM_COL32(120, 84, 40, 235), GREY = IM_COL32(150, 136, 112, 140), GREEN = IM_COL32(40, 92, 34, 235), RED = IM_COL32(112, 40, 22, 235);
    char b[160];
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
    float rh = n ? fminf(32.0f, (BY - TY - 2) / n) : 30;
    float y = TY + 1;
    for (int i = 0; i < 5; i++) {
        const LodgeMember &p = m.m[i];
        if (!p.present) continue;
        bool sel = m.sel == i, hov = P.over(LX0 + 1, y, LX1 - 1, y + rh - 1);
        if (sel) { P.rect(LX0 + 1, y, LX1 - 1, y + rh - 1, SEL, 1.2f); P.rect(LX0 + 1, y, LX0 + 2.6f, y + rh - 1, IM_COL32(140, 70, 20, 230)); }
        else if (hov) P.rect(LX0 + 1, y, LX1 - 1, y + rh - 1, HOVER, 1.2f);
        if (P.hit(LX0 + 1, y, LX1 - 1, y + rh - 1)) ns_push(LV_SELECT, i, 0, 0);
        float nx = LX0 + 5;
        {                                                   /* the member's figure in their colours */
            char code[8]; unsigned char pal[24];
            gm_member_figure_code(i, code, sizeof code); gm_member_palette(i, pal);
            if (const GamePic *g = code[0] ? ns_figure_pic(60 + i, code, pal, "SHORT") : nullptr) {
                float fh = fminf(rh - 4, 26.0f), fw = g->w * fh / (g->h ? g->h : 1);
                P.dl->AddImage((ImTextureID)(intptr_t)g->tex, NsDraw::w(LX0 + 4, y + 2), NsDraw::w(LX0 + 4 + fw, y + 2 + fh));
                nx = LX0 + 7 + fw;
            }
        }
        P.text(f, NsDraw::FS_NAME, nx, y + 1.5f, ink, ns_gtext(p.name), -1, LX0 + 62);
        /* strength and divine favor, small bars */
        auto bar = [&](float x, float yy, const char *lab, int cur, int max, ImU32 col) {
            P.text(fo, 3.8f, x, yy - 0.6f, dim, lab);
            float bx = x + 9, bw = 30;
            P.rect(bx, yy, bx + bw, yy + 3, IM_COL32(200, 186, 160, 255), 0.8f);
            if (max > 0) P.rect(bx, yy, bx + bw * (cur < 0 ? 0 : cur > max ? 1 : (float)cur / max), yy + 3, col, 0.8f);
            char t[24]; snprintf(t, sizeof t, "%d/%d", cur, max);
            P.text(fo, 3.8f, bx + bw + 1.5f, yy - 0.6f, soft, t);
        };
        bar(LX0 + 64, y + 2.2f, "Str", p.str, p.str_max, IM_COL32(70, 150, 70, 255));
        bar(LX0 + 64, y + 7.6f, "DF", p.df, p.df_max, IM_COL32(200, 160, 40, 255));
        std::string line = ns_gtext(p.line);
        if (p.act == 6 && p.skill >= 0 && p.skill < 19) {
            int fee = 0; for (int t = 0; t < m.nteach; t++) if (m.t[t].skill == p.skill) fee = m.t[t].fee;
            snprintf(b, sizeof b, "Train or Study: %s (%s a day)", SKILL[p.skill], money_str(fee).c_str());
            line = b;
        }
        P.para(fo, NsDraw::FS_BODY, nx, y + 12.5f, LX1 - nx - 4, p.act ? IM_COL32(92, 40, 12, 255) : soft, line);
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
        static const char *WHY[7] = { "", "Already at full strength.", "Divine favor is full.", "Knows no formula to make.", "Only at an inn in a town.", "Only when camping in the wild.", "No teacher in this place now." };
        static const char *WHAT[7] = { "Rest: endurance comes back.", "Rest and care: strength comes back, the more the better the party's healing.",
            "Prayer: divine favor comes back, the more the better religion and virtue.", "Make potions from a formula known (the ingredients are used up).",
            "Work in the town for a wage, by the member's skills.", "Keep watch and find food: the camp is safer.", "Lessons from a teacher of the town, for a fee each day." };
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
                    bool tc = p.act == 6 && p.skill == T.skill, th = P.over(x0 + 6, ay, x1, ay + 8);
                    P.rect(x0 + 6, ay, x1, ay + 8, tc ? IM_COL32(140, 70, 20, 235) : th ? IM_COL32(214, 190, 146, 255) : IM_COL32(236, 224, 196, 255), 1.0f);
                    P.text(fo, NsDraw::FS_BODY, x0 + 9, ay + 1.2f, tc ? NsDraw::LIGHT : ink, T.skill >= 0 && T.skill < 19 ? SKILL[T.skill] : "?");
                    snprintf(b, sizeof b, "%s a day", money_str(T.fee).c_str());
                    P.text(fo, NsDraw::FS_SMALL, 0, ay + 1.8f, tc ? NsDraw::LIGHT : soft, b, x1 - 2);
                    if (th) hover_act = 100 + t;
                    if (P.hit(x0 + 6, ay, x1, ay + 8) && !tc) ns_push(LV_TEACH, t, m.sel, 0);
                    ay += 8.6f;
                }
            }
        }
        /* about the one under the pointer */
        std::string about;
        if (hover_act >= 0 && hover_act < 7) about = std::string(WHAT[hover_act]) + (p.can[hover_act] ? "" : std::string(" ") + WHY[hover_act]);
        else if (hover_act >= 100) {
            const auto &T = m.t[hover_act - 100];
            const char *sk = T.skill >= 0 && T.skill < 19 ? SKILL[T.skill] : "?";
            snprintf(b, sizeof b, "A teacher of %s: %s a day, paid each day of lessons. %s's %s is now %d.", sk, money_with_total(T.fee).c_str(),
                     ns_gtext(p.name).c_str(), sk, T.skill >= 0 && T.skill < 19 ? gm_member_skill(m.sel, T.skill) : 0);
            about = b;
        } else if (m.nteach == 0 && m.mode != 1) about = "No teacher offers lessons in this town now. Teachers are found by asking for tutoring or instruction at the monastery, the university, a guild, the armourer, swordsmith, blacksmith, bowyer, artificer or the town council; they then teach here for a while.";
        if (!about.empty()) P.para(fo, NsDraw::FS_SMALL, RX0 + 4, fmaxf(ay + 2, BY - 22), RX1 - RX0 - 8, soft, about);
    }

    /* the day's cost, the purse; the buttons */
    long after = m.purse - m.cost;
    if (m.mode == 1) snprintf(b, sizeof b, "Purse: %s.", money(m.purse).c_str());
    else if (m.cost > m.purse) snprintf(b, sizeof b, "Purse: %s. A day costs %s in all: not enough money.", money(m.purse).c_str(), money(m.cost).c_str());
    else snprintf(b, sizeof b, "Purse: %s. A day: %s %s in all, leaving %s.", money(m.purse).c_str(), m.cost < 0 ? "earns" : "costs", money(m.cost < 0 ? -m.cost : m.cost).c_str(), money(after).c_str());
    P.para(fo, NsDraw::FS_BODY, 64, 172, 133, m.mode != 1 && m.cost > m.purse ? NsDraw::RED : ink, b);
    float bx = 199;
    if (P.button(bx, 174, bx + 22, 184, "A day", GREEN)) ns_push(LV_DAY, 0, 0, 0);
    if (P.button(bx + 24, 174, bx + 48, 184, "A week", GREEN)) ns_push(LV_DAYS, 7, 0, 0);
    if (P.button(bx + 50, 174, bx + 84, 184, "Until rested", m.resting ? BROWN : GREY) && m.resting) ns_push(LV_REST, 0, 0, 0);
    if (P.over(bx + 50, 174, bx + 84, 184))
        P.para(fo, 3.9f, 190, 186, 126, soft, m.resting ? "Days until no one is regaining strength or praying any more (at most 14; it stops if anything happens)." : "Nobody is regaining strength or praying.");
    else if (P.over(bx + 24, 174, bx + 48, 184))
        P.para(fo, 3.9f, 190, 186, 126, soft, "Seven days as chosen (it stops if anything happens, or when a day cannot be paid).");
    else P.text(fo, 3.9f, 190, 188, dim, "Keys as before: J R P A E G T, S a day, L leave, 1-5 a member");
    if (P.button(bx + 86, 174, bx + 117, 184, "Leave", RED)) ns_push(LV_LEAVE, 0, 0, 0);
}
