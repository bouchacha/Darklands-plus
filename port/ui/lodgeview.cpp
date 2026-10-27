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
#include "../runtime/game.h"
#include "../mods/lodgeview.h"
#include "../mods/gamemodel.h"
#include "money.h"
#include "gamedata.h"
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
/* the activities' pictures: a tankard, a bed, a rosary, the Stone, a few small coins, the watch's lantern, the lectern */
static void act_icon(ImDrawList *dl, ImVec2 c, float s, int a);
void ui_lodge_act_icon(ImDrawList *dl, ImVec2 c, float s, int a) { act_icon(dl, c, s, a); }   /* (the icon sheet, uiconstyles:2) */
static void act_icon(ImDrawList *dl, ImVec2 c, float s, int a)
{
    float u = s / 20.0f;
    auto Q = [&](float x, float y) { return ImVec2(c.x + x * u, c.y + y * u); };
    switch (a) {
    case 0:                                                    /* Regain Endurance: a tankard of ale */
        dl->AddRectFilled(Q(-6, -6), Q(5, 9), IM_COL32(140, 96, 52, 255), 1.5f * u);
        for (int i = 0; i < 3; i++) dl->AddLine(Q(-6, -3 + i * 5), Q(5, -3 + i * 5), IM_COL32(70, 70, 76, 255), 1.6f * u);
        dl->AddEllipseFilled(Q(-0.5f, -6.5f), ImVec2(6.2f * u, 2.6f * u), IM_COL32(250, 244, 226, 255));
        dl->AddEllipseFilled(Q(-3, -8), ImVec2(3 * u, 2 * u), IM_COL32(250, 244, 226, 255));
        dl->PathArcTo(Q(5, 1.5f), 4.2f * u, -1.4f, 1.4f, 10); dl->PathStroke(IM_COL32(70, 70, 76, 255), 0, 2 * u);
        break;
    case 1: {                                                  /* Regain Strength: a bed, its pillow and blanket */
        const ImU32 WOOD = IM_COL32(120, 76, 38, 255), WOOD_D = IM_COL32(78, 48, 22, 255);
        dl->AddRectFilled(Q(-9, -6), Q(-6.5f, 8), WOOD, 0.8f * u);                    /* the headboard */
        dl->AddRectFilled(Q(6.8f, -1), Q(9, 8), WOOD, 0.8f * u);                       /* the foot */
        dl->AddRectFilled(Q(-7, 1.5f), Q(7.5f, 5), WOOD_D);                            /* the frame */
        dl->AddEllipseFilled(Q(-3.8f, -0.6f), ImVec2(3.0f * u, 1.9f * u), IM_COL32(248, 242, 226, 255));   /* the pillow */
        dl->AddRectFilled(Q(-1.2f, -1.6f), Q(7.6f, 2.2f), IM_COL32(150, 44, 32, 255), 1.2f * u);         /* the blanket */
        dl->AddLine(Q(-1.2f, 0.2f), Q(7.6f, 0.2f), IM_COL32(220, 170, 90, 255), 0.7f * u);
        break; }
    case 2: {                                                  /* Pray: a rosary, its beads in a ring and the cross hanging */
        const ImU32 BEAD = IM_COL32(110, 64, 30, 255), CROSS = IM_COL32(190, 150, 60, 255);
        for (int i = 0; i < 12; i++) {
            float an = -1.5708f + 0.45f + i * (6.2832f - 0.9f) / 11;
            dl->AddCircleFilled(Q(cosf(an) * 6.2f, -2 + sinf(an) * 5.4f), 1.25f * u, BEAD, 8);
        }
        dl->AddLine(Q(-1.9f, 3.2f), Q(0, 5), BEAD, 0.8f * u); dl->AddLine(Q(1.9f, 3.2f), Q(0, 5), BEAD, 0.8f * u);
        dl->AddRectFilled(Q(-0.7f, 5), Q(0.7f, 10), CROSS); dl->AddRectFilled(Q(-2.3f, 6.4f), Q(2.3f, 7.7f), CROSS);
        break; }
    case 3: {                                                  /* Alchemy: the party's Philosopher's Stone (ui/pstone.cpp), without its number */
        void ui_pstone_draw(ImDrawList *dl, ImVec2 c, float s, int look, int quality, float t);
        int q = (s16)g_rw(0x9078);
        ui_pstone_draw(dl, c, s * 0.92f, 2, q > 0 ? -q : -1, (float)ImGui::GetTime());
        break; }
    case 4: {                                                  /* Earn a little money: a day's wage, not a fortune (a quest's reward is the purse,
                                                                  ui/questicons.cpp): a few small coins dropped in a heap, one over another,
                                                                  silver groschen and copper pfennigs, drawn as the money's coins are (ui/money.cpp) */
        dl->AddEllipseFilled(Q(0.4f, 7.6f), ImVec2(8.6f * u, 2.4f * u), IM_COL32(40, 24, 10, 70));   /* (their shadow on the table) */
        money_coin_draw(dl, Q(3.4f, 0.6f), 3.5f * u, 2);       /* (the back: a pfennig) */
        money_coin_draw(dl, Q(-2.7f, 1.6f), 4.0f * u, 1);      /* (a groschen over its edge) */
        money_coin_draw(dl, Q(4.0f, 5.0f), 3.3f * u, 2);       /* (a pfennig fallen to the right) */
        money_coin_draw(dl, Q(-0.3f, 5.4f), 4.0f * u, 1);      /* (and a groschen on top) */
        break; }
    case 5: {                                                  /* Guard the camp: the watch's lantern hung on its staff, lit */
        const ImU32 IRON = IM_COL32(62, 58, 56, 255), WOOD = IM_COL32(120, 76, 38, 255), GLOW = IM_COL32(255, 214, 110, 255);
        for (int i = 4; i >= 1; i--) dl->AddCircleFilled(Q(2.2f, 3.4f), (4.5f + i * 1.6f) * u, IM_COL32(255, 196, 90, 18 + 6 * (4 - i)), 16);   /* its light */
        dl->AddLine(Q(-7.5f, 10), Q(-7.5f, -9.5f), WOOD, 1.9f * u);                       /* the staff */
        dl->AddLine(Q(-8.2f, -9.2f), Q(2.2f, -9.2f), WOOD, 1.7f * u);                      /* its arm */
        dl->AddLine(Q(2.2f, -9.2f), Q(2.2f, -6.2f), IRON, 1.0f * u);                       /* the hook */
        dl->AddTriangleFilled(Q(-1.3f, -3.2f), Q(5.7f, -3.2f), Q(2.2f, -6.4f), IRON);       /* the cap */
        dl->AddRectFilled(Q(-0.8f, -3.2f), Q(5.2f, 9.0f), GLOW, 0.6f * u);                 /* the glass, lit */
        dl->AddCircleFilled(Q(2.2f, 3.6f), 1.5f * u, IM_COL32(255, 250, 220, 255), 8);       /* the flame */
        dl->AddRect(Q(-0.8f, -3.2f), Q(5.2f, 9.0f), IRON, 0.6f * u, 0, 1.1f * u);          /* the frame */
        dl->AddLine(Q(2.2f, -3.2f), Q(2.2f, 9.0f), IRON, 0.8f * u);
        dl->AddRectFilled(Q(-1.6f, 8.6f), Q(6.0f, 10.2f), IRON, 0.5f * u);                 /* the base */
        break; }
    default: {                                                 /* Train or study: the teachers' lectern, as on the cards (ui/teachersui.cpp) */
        void ui_draw_teach_mark(ImDrawList *dl, ImVec2 c, float s, unsigned ink, unsigned page);
        ui_draw_teach_mark(dl, Q(0, -0.5f), s * 0.98f, IM_COL32(80, 50, 24, 255), IM_COL32(252, 246, 228, 255));
        break; }
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
    if (!camp) {                                             /* (in camp the risk is told under Guard the camp) */
        snprintf(b, sizeof b, "The room: %s a day.%s", money(m.lodging).c_str(), m.days ? ("   Days stayed: " + std::to_string(m.days)).c_str() : "");
        P.text(fo, NsDraw::FS_SMALL, 70, 23.5f, soft, b, -1, 186);
    }

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
        } else if (act == 3 && p.nbrew > 0) {                            /* the potions by name, not the game's "Make 3 potions" */
            line = "Makes ";
            for (int k = 0; k < p.nbrew; k++) {
                int fm = p.brew_f[k];
                line += (k ? ", " : "") + std::to_string(p.brew_n[k]) + " " + (fm >= 0 && fm < (int)gamedata.formula_names.size() ? gamedata.formula_names[fm] : std::string("?"));
            }
        } else if (act == 0) { line = "Relaxes"; more = p.end < p.end_max ? "Endurance back to full (" + std::to_string(p.end) + " of " + std::to_string(p.end_max) + ")" : p.can[4] ? "Already rested: earning would be better" : "Already rested"; if (p.end >= p.end_max && p.can[4]) mc = IM_COL32(150, 70, 20, 255); }
        else if (act == 1) more = "Strength " + std::to_string(p.str) + " of " + std::to_string(p.str_max);
        else if (act == 5) { line = "Keeps watch"; more = "Woodwise " + std::to_string(gm_member_skill(i, 18)) + ", Stealth " + std::to_string(gm_member_skill(i, 15)); }
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
        static const char *ACT[7] = { "Regain Endurance", "Regain Strength", "Pray for Divine Favor", "Alchemy work", "Earn a little money", "Guard the camp", "Train or study" };
        static const char *WHY[7] = { "Endurance is full: earning money is the better day.", "Already at full Strength.", "Divine Favor is full.", "Knows no formula to make.", "Only at an inn in a town.", "Only when camping in the wild.", "No teachers retained in this town." };
        static const char *WHAT[7] = { "Rest: Endurance comes back to full. Nothing else is gained.", "Rest and care: Strength comes back; the better the party's Healing, the more (Endurance too).",
            "Prayer: Divine Favor comes back; the better Religion and Virtue, the more. Endurance does not come back that day.", "Make potions from a known formula (the ingredients are used up). Endurance does not come back that day.",
            "Work in the town: the job that pays this member best, by their attributes and skills (wounds lower the pay; a bad name in the town too). Endurance does not come back that day.",
            "Keep watch: the risk that the lord's men find the party living off his land grows more slowly each day. Woodwise helps most, then Stealth, then a weapon skill of 30 or more.", "Lessons from a teacher hired in this town, for a fee each day. Endurance does not come back that day." };
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
            if (a == 0 && p.end >= p.end_max && p.can[4]) can = false;   /* (rested, with work to be had: nothing to regain) */
            if (camp && (a == 4 || a == 6)) continue;                 /* (no work and no teachers in the wild; no guard at an inn) */
            if (!camp && a == 5) continue;
            float x0 = RX0, x1 = RX1;
            bool h = P.over(x0, ay, x1, ay + ah - 0.6f);
            if (h) hover_act = a;
            if (cur && a != 6) P.rect(x0, ay, x1, ay + ah - 0.6f, CHOSEN, 1.2f);
            else if (h && can) P.rect(x0, ay, x1, ay + ah - 0.6f, HOVER, 1.2f);
            act_icon(P.dl, NsDraw::w(x0 + 4, ay + ah * 0.48f), ah * 0.72f * P.k, a);
            P.text(f, NsDraw::FS_NAME * fminf(1.0f, ah / 9.4f) * 0.95f, x0 + 10, ay + ah * 0.16f, cur && a != 6 ? NsDraw::LIGHT : can ? ink : dim, ACT[a]);
            if (!can && (WHY[a][0] || a == 0)) P.text(fo, 3.6f, 0, ay + ah * 0.3f, dim, a <= 2 ? "full" : a == 3 ? "no formula" : a == 6 ? "none retained" : "", x1 - 1.5f);
            if (can && a != 6 && P.hit(x0, ay, x1, ay + ah - 0.6f) && !cur) ns_push(LV_ACT, a, m.sel, 0);
            ay += ah;
            if (a == 5 && camp) {                                    /* the risk, as a chance: before the next day, and the day after */
                ImU32 rc = m.risk_pct >= 50 ? NsDraw::RED : m.risk_pct >= 20 ? IM_COL32(150, 70, 20, 255) : soft;
                if (m.risk_add == 0) snprintf(b, sizeof b, "No risk here: the lord's men will not come.");
                else snprintf(b, sizeof b, "The lord's men: %d%% before the next day, %d%% the day after", m.risk_pct, m.risk_next_pct);
                P.text(fo, NsDraw::FS_SMALL * fminf(1.0f, ah / 9.4f), x0 + 10, ay - 1.2f, rc, b, -1, x1 - 1);
                if (P.over(x0, ay - 1.4f, x1, ay + 5.4f)) hover_act = 200;
                ay += 6.4f;
            }
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
                    if (capped) snprintf(b, sizeof b, "learned all (%d)", T.val);
                    else snprintf(b, sizeof b, "now %d, to %d  \xc2\xb7  %d%%  \xc2\xb7  %s  \xc2\xb7  %d day%s", gm_member_skill(m.sel, T.skill), T.val, teach_chance(T.val), money_str(T.fee).c_str(), T.days_left, T.days_left == 1 ? "" : "s");
                    P.text(fo, NsDraw::FS_SMALL * fminf(1.0f, th / 9.0f), 0, ay + th * 0.2f, scol, b, x1 - 1.5f);
                    if (thv) hover_act = 100 + t;
                    if (P.hit(x0 + 6, ay, x1, ay + th - 0.6f) && !tc && !capped) ns_push(LV_TEACH, t, m.sel, 0);
                    ay += th;
                }
            }
        }
        /* about the one under the pointer, in its own space at the page's foot */
        std::string about;
        if (hover_act == 200) {
            char b[400];
            int g = 0; for (int i = 0; i < 5; i++) g += m.m[i].present && m.m[i].act == 5;
            if (g) snprintf(b, sizeof b, "The lord's men may find the party living off his land, more likely each day. With %s on guard the day after is %d%% (%d%% unguarded). Camp broken, the risk stays half a day to two and a half.",
                            g == 1 ? "this one" : "these", m.risk_next_pct, m.risk_next_unguarded_pct);
            else snprintf(b, sizeof b, "The lord's men may find the party living off his land, more likely each day; members on guard slow it. Camp broken, the risk stays half a day to two and a half.");
            about = b;
        } else if (hover_act >= 0 && hover_act < 7) about = std::string(WHAT[hover_act]) + (p.can[hover_act] && !(hover_act == 0 && p.end >= p.end_max && p.can[4]) ? "" : std::string(" ") + WHY[hover_act]);
        if (hover_act == 3 && !camp) about += " At an inn a failed batch can blow up; in a camp in the wild it cannot.";
        else if (hover_act >= 100 && hover_act < 200) {
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
        } else if (hover_act < 0) about = m.nteach == 0 && !camp ? "No teachers retained in this town: ask a guild's masters, the university, the monks, the physician or an alchemist (with the Darklands+ switch \"Teachers\" on, a lectern marks the option on their cards); a teacher stays a week."
                                                               : "Point at a choice to see what it does.";
        P.para(fo, NsDraw::FS_SMALL, RX0, BY - 25, RX1 - RX0, hover_act < 0 ? dim : soft, about);
    }

    /* on the table: the day's reckoning as a ledger (the purse, what comes in, what goes out, a
       line, what is left), and the buttons */
    {
        long fees = 0, wages = 0;
        for (int i = 0; i < 5; i++) {
            const LodgeMember &p = m.m[i];
            if (!p.present) continue;
            if (p.act == 6) { for (int t = 0; t < m.nteach; t++) if (m.t[t].skill == p.skill && m.t[t].used) fees += m.t[t].fee; }
            if (p.act == 4) wages += p.val;
        }
        long out = m.lodging + fees, after = m.purse - m.cost;
        bool short_ = m.cost > m.purse;
        P.rect(64, 172, 196, 197, IM_COL32(236, 222, 188, 235), 1.5f);
        const float LX = 67, RX_ = 128;                       /* the labels; the amounts' right edge */
        auto row = [&](float y, const char *sign, const std::string &label, long v, ImU32 c) {
            P.text(fo, NsDraw::FS_SMALL, LX, y, c, std::string(sign) + label);
            P.text(fo, NsDraw::FS_SMALL, 0, y, c, v ? money(v) : std::string("-"), RX_);
        };
        row(173.0f, "", "In the purse", m.purse, soft);
        row(178.4f, "+ ", "Wages", wages, wages ? GREEN : dim);
        row(183.8f, "- ", camp ? (fees ? "Lessons" : "Food: from the land") : fees ? "Room and lessons" : "Room", out, out ? RED : dim);
        P.rect(LX, 189.6f, RX_, 189.9f, IM_COL32(120, 80, 36, 160));
        P.text(fo, NsDraw::FS_SMALL, LX, 190.6f, ink, "After a day");
        P.text(fo, NsDraw::FS_SMALL, 0, 190.6f, short_ ? NsDraw::RED : ink, short_ ? std::string("cannot pay") : money(after), RX_);
        /* at the right: the day's sum alone, large */
        P.rect(132, 174, 132.3f, 195, IM_COL32(120, 80, 36, 90));
        const char *hd = m.cost < 0 ? "A day earns" : m.cost > 0 ? "A day costs" : "A day costs";
        P.text(fo, NsDraw::FS_SMALL, 136, 175, soft, hd);
        std::string amt = m.cost ? money(m.cost < 0 ? -m.cost : m.cost) : std::string("nothing");
        P.text(fo, NsDraw::FS_HEAD, 136, 182, short_ ? NsDraw::RED : m.cost < 0 ? GREEN : ink, amt);
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
