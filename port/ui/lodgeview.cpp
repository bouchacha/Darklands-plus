// lodgeview.cpp - an inn's room or a camp drawn by Darklands+ (mods/lodgeview.c runs it): the
// members with each one's day, what the chosen member can do (the original's words; greyed with
// the reason where not), the town's teachers with their fees, what a day costs against the purse,
// and Spend a day, Until rested and Leave.
#include <stdio.h>
#include <string>
#include "nsdraw.h"
#include <math.h>
#include "drawcache.h"
#include "alchemydesk.h"
#include "skillicons.h"
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

/* ---------------------------------------------------------------- the room and the ledger
   An inn: the common room's oak table, the stay kept in a ledger bound in brown leather (the party's
   days on its left page, the chosen member's choices on its right), a sign over it with the inn's
   town, a candle; the reckoning and the buttons on the table below. A camp: the same ledger on a
   blanket by night, the fire's glow. */
static void candle(ImDrawList *dl, ImVec2 c, float u, float t)
{
    dl->AddRectFilled(ImVec2(c.x - 2.2f * u, c.y - 7 * u), ImVec2(c.x + 2.2f * u, c.y + 3 * u), IM_COL32(236, 226, 196, 255), 0.8f * u);
    dl->AddRectFilledMultiColor(ImVec2(c.x - 2.2f * u, c.y - 7 * u), ImVec2(c.x, c.y + 3 * u), IM_COL32(255, 250, 230, 120), 0, 0, IM_COL32(255, 250, 230, 120));
    dl->AddEllipseFilled(ImVec2(c.x, c.y + 3.2f * u), ImVec2(4.6f * u, 1.6f * u), IM_COL32(160, 120, 60, 255));     /* (the holder) */
    float fl = 1 + 0.12f * sinf(t * 9.0f) + 0.06f * sinf(t * 23.0f);
    for (int i = 10; i >= 1; i--) { float f = i / 10.0f; dl->AddCircleFilled(ImVec2(c.x, c.y - 10 * u), 16 * u * f, IM_COL32(255, 200, 110, (int)(10 * (1 - f) + 2))); }
    dl->AddEllipseFilled(ImVec2(c.x, c.y - 9.6f * u), ImVec2(1.3f * u, 2.6f * u * fl), IM_COL32(255, 190, 70, 240));
    dl->AddEllipseFilled(ImVec2(c.x, c.y - 9.0f * u), ImVec2(0.6f * u, 1.4f * u * fl), IM_COL32(255, 246, 200, 255));
    dl->AddLine(ImVec2(c.x, c.y - 7 * u), ImVec2(c.x, c.y - 8 * u), IM_COL32(40, 30, 20, 255), 0.7f * u);
}
static void backdrop(NsDraw &P, int camp)
{
    ImVec2 o = NsDraw::w(60, 0), e = NsDraw::w(320, 200);
    dc_draw(P.dl, o, e, 0x10d6e00ull + (unsigned)camp, [&](ImDrawList *dl) {
        if (camp) {                                            /* the ground by night, a dark blanket, the fire's light from the lower left */
            dl->AddRectFilledMultiColor(o, e, IM_COL32(26, 30, 38, 255), IM_COL32(20, 22, 30, 255), IM_COL32(30, 22, 14, 255), IM_COL32(56, 34, 18, 255));
            for (int i = 0; i < 260; i++) {
                unsigned h = (unsigned)i * 2654435761u; h ^= h >> 13;
                float x = 60 + (h & 1023) / 1023.0f * 260, y = 30 + ((h >> 10) & 1023) / 1023.0f * 170;
                dl->AddCircleFilled(NsDraw::w(x, y), 0.6f * P.k, IM_COL32(70, 60, 40, 90), 5);
            }
            dl->AddRectFilled(NsDraw::w(62, 21), NsDraw::w(318, 170), IM_COL32(64, 46, 34, 255), 3 * P.k);      /* the blanket */
            for (int i = 0; i < 12; i++) dl->AddLine(NsDraw::w(62, 26 + i * 12.4f), NsDraw::w(318, 26 + i * 12.4f), IM_COL32(110, 40, 30, 120), 1.6f * P.k);
            for (int i = 0; i < 20; i++) dl->AddLine(NsDraw::w(66 + i * 13, 21), NsDraw::w(66 + i * 13, 170), IM_COL32(30, 20, 14, 60), 0.8f * P.k);
            for (int i = 14; i >= 1; i--) { float f = i / 14.0f; dl->AddEllipseFilled(NsDraw::w(70, 200), ImVec2(160 * P.kx * f, 120 * P.k * f), IM_COL32(255, 140, 50, (int)(30 * (1 - f)))); }
        } else desk_table(dl);
        desk_book(dl, 63, 20, 317, 169, 189, camp ? IM_COL32(70, 54, 30, 255) : IM_COL32(110, 70, 34, 255));
        /* ruled lines, faint, on both pages */
        for (float y = 34; y < 164; y += 9.5f) {
            dl->AddLine(NsDraw::w(70, y), NsDraw::w(184, y), IM_COL32(150, 110, 60, 28), 1.0f);
            dl->AddLine(NsDraw::w(194, y), NsDraw::w(311, y), IM_COL32(150, 110, 60, 28), 1.0f);
        }
        if (!camp) {                                           /* the sign over the table: a board hung on two chains */
            ImVec2 a = NsDraw::w(98, 2), b = NsDraw::w(282, 16);
            for (int s = 0; s < 2; s++) { float x = s ? 262 : 118; dl->AddLine(NsDraw::w(x, -1), NsDraw::w(x, 3), IM_COL32(40, 36, 30, 255), 1.5f * P.k); }
            dl->AddRectFilled(ImVec2(a.x + 2 * P.k, a.y + 2 * P.k), ImVec2(b.x + 2 * P.k, b.y + 2 * P.k), IM_COL32(0, 0, 0, 110), 2 * P.k);
            dl->AddRectFilledMultiColor(a, b, IM_COL32(126, 82, 42, 255), IM_COL32(112, 72, 36, 255), IM_COL32(74, 46, 22, 255), IM_COL32(84, 52, 26, 255));
            dl->AddRect(a, b, IM_COL32(40, 22, 8, 255), 2 * P.k, 0, 1.4f * P.k);
            dl->AddRect(NsDraw::w(99.5f, 3.5f), NsDraw::w(280.5f, 14.5f), IM_COL32(214, 170, 80, 150), 1.5f * P.k, 0, 0.8f * P.k);
        }
    });
}
/* the activities' pictures: a tankard, the skills' and attributes' own, a purse */
static void act_icon(ImDrawList *dl, ImVec2 c, float s, int a)
{
    float u = s / 20.0f;
    auto Q = [&](float x, float y) { return ImVec2(c.x + x * u, c.y + y * u); };
    switch (a) {
    case 0:                                                    /* Just Relax: a tankard of ale */
        dl->AddRectFilled(Q(-6, -6), Q(5, 9), IM_COL32(140, 96, 52, 255), 1.5f * u);
        for (int i = 0; i < 3; i++) dl->AddLine(Q(-6, -3 + i * 5), Q(5, -3 + i * 5), IM_COL32(70, 70, 76, 255), 1.6f * u);
        dl->AddEllipseFilled(Q(-0.5f, -6.5f), ImVec2(6.2f * u, 2.6f * u), IM_COL32(250, 244, 226, 255));
        dl->AddEllipseFilled(Q(-3, -8), ImVec2(3 * u, 2 * u), IM_COL32(250, 244, 226, 255));
        dl->PathArcTo(Q(5, 1.5f), 4.2f * u, -1.4f, 1.4f, 10); dl->PathStroke(IM_COL32(70, 70, 76, 255), 0, 2 * u);
        break;
    case 1: ui_attr_icon(dl, c, s * 0.62f, 1); break;                  /* Regain Strength */
    case 2: ui_attr_icon(dl, c, s * 0.62f, 6); break;                  /* Pray: Divine Favor's chalice */
    case 3: ui_skill_icon(dl, c, s * 0.62f, 7); break;                 /* Alchemy */
    case 4:                                                    /* Earn a little money: a purse and coins */
        dl->AddCircleFilled(Q(0, 3), 7 * u, IM_COL32(150, 100, 52, 255), 20);
        dl->AddTriangleFilled(Q(-4, -3), Q(4, -3), Q(0, 1), IM_COL32(150, 100, 52, 255));
        dl->AddLine(Q(-4, -3.5f), Q(4, -3.5f), IM_COL32(90, 50, 20, 255), 1.6f * u);
        dl->AddEllipseFilled(Q(-3, -6), ImVec2(2.2f * u, 1.6f * u), IM_COL32(150, 100, 52, 255));
        dl->AddEllipseFilled(Q(3, -6), ImVec2(2.2f * u, 1.6f * u), IM_COL32(150, 100, 52, 255));
        dl->AddCircleFilled(Q(6, 7), 3.2f * u, IM_COL32(226, 182, 64, 255), 14); dl->AddCircle(Q(6, 7), 3.2f * u, IM_COL32(150, 110, 30, 255), 14, 0.8f * u);
        dl->AddCircleFilled(Q(-6, 8), 2.8f * u, IM_COL32(200, 200, 208, 255), 14); dl->AddCircle(Q(-6, 8), 2.8f * u, IM_COL32(120, 120, 128, 255), 14, 0.8f * u);
        break;
    case 5: ui_skill_icon(dl, c, s * 0.62f, 3); break;                 /* Guard the camp: a polearm */
    default:                                                   /* Train or study: an open book */
        dl->AddQuadFilled(Q(-9, -5), Q(0, -3), Q(0, 8), Q(-9, 6), IM_COL32(246, 236, 210, 255));
        dl->AddQuadFilled(Q(0, -3), Q(9, -5), Q(9, 6), Q(0, 8), IM_COL32(236, 224, 194, 255));
        dl->AddQuad(Q(-9, -5), Q(0, -3), Q(0, 8), Q(-9, 6), IM_COL32(90, 60, 30, 255), 0.8f * u);
        dl->AddQuad(Q(0, -3), Q(9, -5), Q(9, 6), Q(0, 8), IM_COL32(90, 60, 30, 255), 0.8f * u);
        for (int i = 0; i < 3; i++) { dl->AddLine(Q(-7, -1.5f + i * 2.5f), Q(-2, -0.6f + i * 2.5f), IM_COL32(120, 90, 60, 200), 0.6f * u); dl->AddLine(Q(2, -0.6f + i * 2.5f), Q(7, -1.5f + i * 2.5f), IM_COL32(120, 90, 60, 200), 0.6f * u); }
        break;
    }
}

extern "C" void lodgeview_draw(void)
{
    const LodgeModel &m = lodge_model;
    if (!lodgeview_showing()) return;
    NsDraw P;
    if (!P.begin()) return;
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM;
    const ImU32 HOVER = IM_COL32(150, 110, 60, 40), SEL = IM_COL32(150, 100, 40, 60), CHOSEN = IM_COL32(120, 52, 18, 235);
    const ImU32 BROWN = IM_COL32(120, 84, 40, 235), GREY = IM_COL32(150, 136, 112, 140), GREEN = IM_COL32(40, 92, 34, 235), RED = IM_COL32(112, 40, 22, 235);
    const ImU32 SIGN = IM_COL32(250, 228, 170, 255), TABLE_INK = IM_COL32(246, 232, 200, 255);
    char b[200];
    if (m.passing) P.click = false;                         /* (the hours going by: a click skips them, mods/lodgeview.c) */
    const int camp = m.mode == 1;
    float tnow = ImGui::GetTime();
    backdrop(P, camp);
    std::string place = ns_gtext(m.place);
    std::string title = camp ? "Camping in the wild" : m.mode == 0 ? (place.empty() ? std::string("At the inn") : "The inn at " + place) : "Staying here";
    {   float tw = P.width(f, NsDraw::FS_TITLE, title), cx = camp ? 190 : 190;
        P.text(f, NsDraw::FS_TITLE, cx - tw * 0.5f, camp ? 4 : 3.6f, camp ? TABLE_INK : SIGN, title); }
    if (!camp) candle(P.dl, NsDraw::w(304, 13), P.k, tnow);
    else {                                                   /* the night over the camp: stars, the fire */
        for (int i = 0; i < 40; i++) {
            unsigned h = (unsigned)i * 2246822519u; h ^= h >> 15;
            float x = 64 + (h & 1023) / 1023.0f * 252, yy = 1 + ((h >> 10) & 1023) / 1023.0f * 17;
            float tw = 0.55f + 0.45f * sinf(tnow * (1.2f + (h >> 20) % 7 * 0.3f) + i);
            P.dl->AddCircleFilled(NsDraw::w(x, yy), (0.35f + ((h >> 24) & 3) * 0.12f) * P.k, IM_COL32(230, 230, 255, (int)(200 * tw)), 6);
        }
        ImVec2 c = NsDraw::w(84, 15); float u = P.k;
        for (int i = 10; i >= 1; i--) { float f = i / 10.0f; P.dl->AddCircleFilled(c, 18 * u * f, IM_COL32(255, 150, 60, (int)(14 * (1 - f) + 2))); }
        P.dl->AddLine(ImVec2(c.x - 7 * u, c.y + 3 * u), ImVec2(c.x + 7 * u, c.y), IM_COL32(90, 56, 28, 255), 2.2f * u);
        P.dl->AddLine(ImVec2(c.x - 7 * u, c.y), ImVec2(c.x + 7 * u, c.y + 3 * u), IM_COL32(110, 70, 34, 255), 2.2f * u);
        for (int j = 0; j < 3; j++) {
            float ph = tnow * (5.0f + j) + j * 2.1f, hgt = (7 + 2.5f * sinf(ph)) * (1 - j * 0.25f), wd = 4.5f - j * 1.2f;
            ImU32 col = j == 0 ? IM_COL32(210, 70, 20, 230) : j == 1 ? IM_COL32(245, 150, 40, 240) : IM_COL32(255, 230, 140, 255);
            P.dl->AddTriangleFilled(ImVec2(c.x - wd * u, c.y + 1.5f * u), ImVec2(c.x + 0.8f * sinf(ph * 1.3f) * u, c.y - hgt * u), ImVec2(c.x + wd * u, c.y + 1.5f * u), col);
        }
    }
    if (camp) snprintf(b, sizeof b, "Living off the land, the lord may hear of it: after %d day%s the risk is %d of 10.", m.days, m.days == 1 ? "" : "s", m.risk);
    else snprintf(b, sizeof b, "The room: %s a day.%s", money(m.lodging).c_str(), m.days ? ("   Days stayed: " + std::to_string(m.days)).c_str() : "");
    P.text(fo, NsDraw::FS_SMALL, 70, 23.5f, soft, b, -1, 186);

    /* the left page: the party's days */
    const float LX0 = 68, LX1 = 185, TY = 30, BY = 164;
    int n = 0; for (int i = 0; i < 5; i++) n += m.m[i].present;
    float rh = n ? fminf(26.0f, (BY - TY) / n) : 26;
    float y = TY;
    for (int i = 0; i < 5; i++) {
        const LodgeMember &p = m.m[i];
        if (!p.present) continue;
        bool sel = m.sel == i, hov = P.over(LX0, y, LX1, y + rh - 1);
        if (sel) { P.rect(LX0 - 1, y, LX1, y + rh - 1, SEL, 1.2f); P.rect(LX0 - 1, y, LX0 + 0.6f, y + rh - 1, IM_COL32(140, 70, 20, 230)); }
        else if (hov) P.rect(LX0 - 1, y, LX1, y + rh - 1, HOVER, 1.2f);
        if (P.hit(LX0, y, LX1, y + rh - 1)) ns_push(LV_SELECT, i, 0, 0);
        int act = p.act >= 0 && p.act <= 6 ? p.act : 0;
        act_icon(P.dl, NsDraw::w(LX0 + 5, y + 11.5f), 8 * P.k, act);
        float nx = LX0 + 13;
        P.text(f, NsDraw::FS_NAME, nx, y + 1.5f, ink, ns_gtext(p.name), -1, LX1 - 30);
        /* what the day does for it: the game's line, made plain */
        std::string line = ns_gtext(p.line), more;
        ImU32 mc = soft;
        if (act == 6 && p.skill >= 0 && p.skill < 19) {
            int ti = -1; for (int t = 0; t < m.nteach; t++) if (m.t[t].skill == p.skill && (m.t[t].used || ti < 0)) ti = t;
            line = std::string("Studies ") + gm_skill_name(p.skill);
            if (ti >= 0) {
                const auto &T = m.t[ti];
                int now = gm_member_skill(i, p.skill), c = teach_chance(T.val);
                snprintf(b, sizeof b, "%d now, %d%% a day for +1, up to %d; %s a day", now, c, T.val, money_str(T.fee).c_str());
                more = b;
            }
        } else if (act == 0) { line = "Relaxes"; more = p.end < p.end_max ? "Endurance back to full (" + std::to_string(p.end) + " of " + std::to_string(p.end_max) + ")" : p.can[4] ? "Already rested: earning would be better" : "Already rested"; if (p.end >= p.end_max && p.can[4]) mc = IM_COL32(150, 70, 20, 255); }
        else if (act == 1) more = "Strength " + std::to_string(p.str) + " of " + std::to_string(p.str_max);
        else if (act == 2) more = "Divine Favor " + std::to_string(p.df) + " of " + std::to_string(p.df_max);
        else if (act == 4) more = p.end < p.end_max ? "Endurance stays down (" + std::to_string(p.end) + " of " + std::to_string(p.end_max) + ")" : "";
        P.text(fo, NsDraw::FS_BODY, nx, y + 8.6f, IM_COL32(92, 40, 12, 255), line, -1, LX1 - 1);
        if (!more.empty() && rh >= 20) P.text(fo, NsDraw::FS_SMALL, nx, y + 15.0f, mc, more, -1, LX1 - 1);
        /* the wounded: End / Str short, on the right of the name */
        std::string shortv;
        if (p.end_max > 0 && p.end < p.end_max) shortv = "End " + std::to_string(p.end);
        if (p.str_max > 0 && p.str < p.str_max) shortv += (shortv.empty() ? "" : "  ") + std::string("Str ") + std::to_string(p.str);
        if (!shortv.empty()) P.text(fo, NsDraw::FS_SMALL, 0, y + 2.2f, NsDraw::RED, shortv, LX1 - 1);
        y += rh;
    }

    /* the right page: the chosen member's choices */
    const float RX0 = 194, RX1 = 312;
    int hover_act = -1;
    if (m.sel >= 0 && m.sel < 5 && m.m[m.sel].present) {
        const LodgeMember &p = m.m[m.sel];
        snprintf(b, sizeof b, "%s's day", ns_gtext(p.name).c_str());
        P.text(f, NsDraw::FS_HEAD, RX0, TY - 1.5f, ink, b);
        static const char *ACT[7] = { "Just Relax", "Regain Strength", "Pray for divine favor", "Alchemy work", "Earn a little money", "Guard the camp", "Train or study" };
        static const char *WHY[7] = { "", "Already at full Strength.", "Divine Favor is full.", "Knows no formula to make.", "Only at an inn in a town.", "Only when camping in the wild.", "No teacher in this place now." };
        static const char *WHAT[7] = { "Rest: Endurance comes back to full. Nothing else is gained.", "Rest and care: Strength comes back; the better the party's Healing, the more (Endurance too).",
            "Prayer: Divine Favor comes back; the better Religion and Virtue, the more. Endurance does not come back that day.", "Make potions from a known formula (the ingredients are used up). Endurance does not come back that day.",
            "Work in the town: the job that pays this member best, by its attributes and skills (wounds lower the pay; a bad name in the town too). Endurance does not come back that day.",
            "Keep watch and find food: the camp is safer.", "Lessons from a teacher hired in this town, for a fee each day. Endurance does not come back that day." };
        /* the teachers, one row a skill: the one who teaches it (the best, with the bestteacher fix) */
        int tl[LV_TEACHERS], ntl = 0;
        for (int t = 0; t < m.nteach; t++) {
            bool dup = false;
            for (int j = 0; j < ntl; j++) if (m.t[tl[j]].skill == m.t[t].skill) { dup = true; if (m.t[t].used && !m.t[tl[j]].used) tl[j] = t; }
            if (!dup) tl[ntl++] = t;
        }
        int rows_t = p.can[6] ? ntl : 0;
        const float LTOP = TY + 8, LBOT = BY - 27;               /* (the list; under it, about the one pointed at) */
        float ah = 9.4f, th = 9.0f;
        float need = 7 * ah + rows_t * th;
        if (need > LBOT - LTOP) { float kk = (LBOT - LTOP) / need; ah *= kk; th *= kk; }
        float ay = LTOP;
        for (int a = 0; a < 7; a++) {
            bool can = p.can[a], cur = p.act == a;
            if (camp && a == 4) continue;                             /* (no work in the wild; no guard at an inn) */
            if (!camp && a == 5) continue;
            float x0 = RX0, x1 = RX1;
            bool h = P.over(x0, ay, x1, ay + ah - 0.6f);
            if (h) hover_act = a;
            if (cur && a != 6) P.rect(x0, ay, x1, ay + ah - 0.6f, CHOSEN, 1.2f);
            else if (h && can) P.rect(x0, ay, x1, ay + ah - 0.6f, HOVER, 1.2f);
            act_icon(P.dl, NsDraw::w(x0 + 4, ay + ah * 0.48f), ah * 0.72f * P.k, a);
            P.text(f, NsDraw::FS_NAME * fminf(1.0f, ah / 9.4f) * 0.95f, x0 + 10, ay + ah * 0.16f, cur && a != 6 ? NsDraw::LIGHT : can ? ink : dim, ACT[a]);
            if (!can && WHY[a][0]) P.text(fo, 3.6f, 0, ay + ah * 0.3f, dim, a == 1 ? "full" : a == 2 ? "full" : a == 3 ? "no formula" : a == 6 ? "no teacher" : "", x1 - 1.5f);
            if (can && a != 6 && P.hit(x0, ay, x1, ay + ah - 0.6f) && !cur) ns_push(LV_ACT, a, m.sel, 0);
            ay += ah;
            if (a == 6 && can) {
                for (int j = 0; j < ntl; j++) {
                    int t = tl[j];
                    const auto &T = m.t[t];
                    bool capped = teach_capped(m.sel, T.skill, T.val) != 0;
                    bool tc = p.act == 6 && p.skill == T.skill, thv = P.over(x0 + 6, ay, x1, ay + th - 0.6f);
                    if (tc) P.rect(x0 + 6, ay, x1, ay + th - 0.6f, CHOSEN, 1.0f);
                    else if (thv && !capped) P.rect(x0 + 6, ay, x1, ay + th - 0.6f, HOVER, 1.0f);
                    ImU32 tcol = tc ? NsDraw::LIGHT : capped ? dim : ink, scol = tc ? IM_COL32(244, 226, 190, 255) : capped ? dim : soft;
                    ui_skill_icon(P.dl, NsDraw::w(x0 + 11, ay + th * 0.48f), th * 0.48f * P.k, T.skill);
                    P.text(fo, NsDraw::FS_BODY * fminf(1.0f, th / 9.0f), x0 + 16, ay + th * 0.14f, tcol, gm_skill_name(T.skill), -1, x0 + 46);
                    if (capped) snprintf(b, sizeof b, "learnt all (%d)", T.val);
                    else snprintf(b, sizeof b, "%d%%  \xc2\xb7  to %d  \xc2\xb7  %s  \xc2\xb7  %d day%s", teach_chance(T.val), T.val, money_str(T.fee).c_str(), T.days_left, T.days_left == 1 ? "" : "s");
                    P.text(fo, NsDraw::FS_SMALL * fminf(1.0f, th / 9.0f), 0, ay + th * 0.2f, scol, b, x1 - 1.5f);
                    if (thv) hover_act = 100 + t;
                    if (P.hit(x0 + 6, ay, x1, ay + th - 0.6f) && !tc && !capped) ns_push(LV_TEACH, t, m.sel, 0);
                    ay += th;
                }
            }
        }
        /* about the one under the pointer, in its own space at the page's foot */
        std::string about;
        if (hover_act >= 0 && hover_act < 7) about = std::string(WHAT[hover_act]) + (p.can[hover_act] ? "" : std::string(" ") + WHY[hover_act]);
        if (hover_act == 3 && !camp) about += " At an inn a failed batch can blow up; in a camp in the wild it cannot.";
        else if (hover_act >= 100) {
            const auto &T = m.t[hover_act - 100];
            const char *sk = gm_skill_name(T.skill);
            int now = T.skill >= 0 && T.skill < 19 ? gm_member_skill(m.sel, T.skill) : 0;
            int c = teach_chance(T.val), days = c > 0 ? (100 + c / 2) / c : 0;
            int same = 0; for (int t = 0; t < m.nteach; t++) same += m.t[t].skill == T.skill;
            if (teach_capped(m.sel, T.skill, T.val))
                snprintf(b, sizeof b, "The teacher of %s knows %d; %s has %d already: nothing more to learn here.", sk, T.val, ns_gtext(p.name).c_str(), now);
            else
                snprintf(b, sizeof b, "Teacher of %s (skill %d), %d more day%s hired. Each day: %d%% chance of +1 (about one point in %d day%s)%s. %s has %d.%s",
                         sk, T.val, T.days_left, T.days_left == 1 ? "" : "s", c, days, days == 1 ? "" : "s", fix_is_on("teachcap") ? ", up to the teacher's skill" : "",
                         ns_gtext(p.name).c_str(), now, same > 1 ? (fix_is_on("bestteacher") ? " (The best of the teachers of it hired here teaches.)" : " (The one hired first teaches.)") : "");
            about = b;
        } else if (hover_act < 0) about = m.nteach == 0 && !camp ? "No teacher in this town now: ask a guild's masters, the university, the monks, the physician or an alchemist (a lectern marks the option on their cards); a teacher stays a week."
                                                               : "Point at a choice to see what it does.";
        P.para(fo, NsDraw::FS_SMALL, RX0, BY - 25, RX1 - RX0, hover_act < 0 ? dim : soft, about);
    }

    /* on the table: the reckoning, the buttons */
    long after = m.purse - m.cost;
    b[0] = 0;
    if (m.cost > m.purse) snprintf(b, sizeof b, "A day costs %s in all: more than the purse holds.", money(m.cost).c_str());
    else if (m.cost) snprintf(b, sizeof b, "A day %s %s; the purse %s %s.", m.cost < 0 ? "earns" : "costs", money(m.cost < 0 ? -m.cost : m.cost).c_str(), m.cost < 0 ? "will hold" : "keeps", money(after).c_str());
    else snprintf(b, sizeof b, "A day costs nothing.");
    P.rect(64, 172, 196, 197, IM_COL32(236, 222, 188, 235), 1.5f);
    P.text(fo, NsDraw::FS_BODY, 67, 173.5f, m.cost > m.purse ? NsDraw::RED : ink, b, -1, 195);
    {   /* the day's sum, piece by piece: the room, the lessons, the wages */
        long fees = 0, wages = 0;
        for (int i = 0; i < 5; i++) {
            const LodgeMember &p = m.m[i];
            if (!p.present) continue;
            if (p.act == 6) { for (int t = 0; t < m.nteach; t++) if (m.t[t].skill == p.skill && m.t[t].used) fees += m.t[t].fee; }
            if (p.act == 4) wages += p.val;
        }
        std::string sum = (camp ? std::string("Food from the land") : "Room " + money(m.lodging));
        if (fees) sum += ",  lessons " + money(fees);
        if (wages) sum += ",  wages earned " + money(wages);
        P.text(fo, NsDraw::FS_SMALL, 67, 181, soft, sum, -1, 195);
        snprintf(b, sizeof b, "In the purse now: %s", money(m.purse).c_str());
        P.text(fo, NsDraw::FS_SMALL, 67, 188, soft, b, -1, 195);
    }
    bool pay = m.cost <= m.purse;
    float bx = 199;
    if (P.button(bx, 174, bx + 22, 184, "A day", pay ? GREEN : GREY) && pay) ns_push(LV_DAY, 0, 0, 0);
    if (P.button(bx + 24, 174, bx + 48, 184, "A week", pay ? GREEN : GREY) && pay) ns_push(LV_DAYS, 7, 0, 0);
    bool hurt = false;                                      /* (someone short of strength who only relaxes: Until rested has them regain it) */
    for (int i = 0; i < 5; i++) if (m.m[i].present && m.m[i].act == 0 && m.m[i].str < m.m[i].str_max && m.m[i].can[1]) hurt = true;
    bool rest = (m.resting || hurt) && pay;
    if (P.button(bx + 50, 174, bx + 84, 184, "Until rested", rest ? BROWN : GREY) && rest) ns_push(LV_REST, 0, 0, 0);
    const char *tip = nullptr;
    if (!pay && P.over(bx, 174, bx + 84, 184)) tip = "The purse cannot pay for a day. Members set to Earn a little money may make up the difference; otherwise, leave.";
    else if (P.over(bx + 50, 174, bx + 84, 184)) tip = m.resting || hurt ? "Days pass until no one is regaining Strength or praying (at most 14; it stops if anything happens)." : "Nobody is short of Strength, regaining it or praying.";
    else if (P.over(bx + 24, 174, bx + 48, 184)) tip = "Seven days, each member doing what was chosen (it stops if anything happens, or when a day cannot be paid).";
    if (tip) P.para(fo, 3.9f, 199, 186.5f, 117, TABLE_INK, tip);
    else P.text(fo, 3.9f, 199, 188, IM_COL32(220, 200, 160, 200), "F11: list of keys");
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
