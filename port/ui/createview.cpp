// createview.cpp - making a character, drawn by Darklands+ (mods/createview.c runs it), over the
// whole screen. Left: the step (name and gender; the childhood; the attribute EPs; the profession;
// the skill EPs). Right: what the step does not show already: the character as it stands, or what
// the childhood or profession under the pointer would bring (the attributes after it, with the
// aging after thirty; each skill's gain and the room the skill EPs have after it), what a
// profession needs (ui/professions.cpp: the game's own test, in words) and what it leads to.
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string>
#include <vector>
#include "nsdraw.h"
#include "professions.h"
#include "townicons.h"
#include "skillicons.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../mods/createview.h"
#include "../mods/gamemodel.h"
u32 plat_ticks_ms(void);
}

static int skill_row(int s) { return s < 7 ? s : s < 13 ? s + 1 : s + 2; }   /* the game's row (gaps 7, 14) */
static int attr_cost(int v) { return v >= 39 ? 3 : v >= 29 ? 2 : 1; }
static int skill_cost(int v) { return v >= 79 ? 3 : v >= 49 ? 2 : 1; }

/* the professions by kind, as the game numbers them (DARKLAND.EXE's table: 0..36) */
static const struct { const char *name; int first, n; } GROUPS[7] = {
    { "Arms", 0, 4 }, { "Nobility", 4, 4 }, { "Church", 8, 8 }, { "Trade", 16, 5 },
    { "Learning", 21, 6 }, { "Crafts", 27, 3 }, { "Common folk", 30, 7 } };

static CgCtx ctx_of(const CreateModel &c)
{
    CgCtx x;
    x.age = c.age; x.child = c.child; x.female = c.female; x.last = c.prof; x.prev = c.prev;
    for (int k = 0; k < 6; k++) x.attr[k] = (signed char)c.attr[k];
    for (int k = 0; k < 19; k++) x.skill[k] = (signed char)c.skill[k];
    for (int k = 0; k < 37; k++) x.taken[k] = (unsigned char)c.taken[k];
    return x;
}
extern "C" int cg_offered_c(int *out)            /* (the test command ucg: the rules' list against the game's) */
{
    std::vector<int> v = cg_prof_offered(ctx_of(create_model));
    for (size_t i = 0; i < v.size(); i++) out[i] = v[i];
    return (int)v.size();
}

/* the game's preview (s037 18c8:1738) of childhood or profession prn (state 1 / 3), or of the
   step being spent (state 2 / 4) */
struct Preview { bool on; int prn; int ep; int attr[6]; int gain[19], room[19]; bool adult; };
static Preview preview(const CreateModel &c, int prn)
{
    Preview p = {};
    if (c.state == 2) prn = c.child; else if (c.state == 4) prn = c.prof;
    p.on = prn >= 0;
    if (prn < 0) prn = 0;
    p.prn = prn;
    const CgProf &ad = c.adult[prn < 37 ? prn : 0], &ch = c.childhood[prn < 6 ? prn : 0];
    if (c.state == 1) p.ep = ch.years;
    else if (c.state == 3) p.ep = ad.years + (c.age >= 25 ? 0 : c.age >= 20 ? 5 : 20);
    else p.ep = c.ep;
    int agk = (c.age - 25) / 5; if (agk < 0) agk = 0; if (agk > 8) agk = 8;
    for (int i = 0; i < 6; i++) {
        int v;
        if (c.state == 2 || c.state == 4 || c.state == 0) v = c.attr[i];
        else if (c.age < 15) v = c.attr[i] + ch.attr[i];
        else if (c.age <= 40 || i >= 2) v = c.attr[i] + c.aging[agk][i] + ad.attr[i];
        else v = c.attr[i] + c.aging[agk][i];
        p.attr[i] = v < 1 ? 1 : v > 99 ? 99 : v;
    }
    p.adult = c.age >= 15;
    for (int i = 0; i < 19; i++) {
        if (c.age >= 25 - (c.state == 3 ? 5 : 0) && c.state != 2) { p.gain[i] = ad.skill[i]; p.room[i] = ad.maxsk[i] - c.skill_spent[i]; }
        else if (c.age >= 15 && c.state != 2) { p.gain[i] = ad.skill[i] + 2; p.room[i] = ad.maxsk[i] - c.skill_spent[i]; }
        else if (c.state == 0) { p.gain[i] = 0; p.room[i] = 0; }
        else { p.gain[i] = ch.skill[i]; p.room[i] = 0; }
    }
    return p;
}

static std::string join_profs(const CreateModel &c, const std::vector<int> &v)
{
    std::string r;
    for (size_t i = 0; i < v.size(); i++) r += (i ? ", " : "") + ns_gtext(c.prof_name[v[i]]);
    return r;
}

extern "C" void createview_draw(void)
{
    const CreateModel &c = create_model;
    if (!c.open || ns_screen != NS_CREATE || ns_hidden()) return;
    NsDraw P;
    if (!P.begin()) return;
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM;
    const ImU32 GREEN = IM_COL32(40, 92, 34, 235), BROWN = IM_COL32(120, 84, 40, 235), RED = IM_COL32(112, 40, 22, 235);
    const ImU32 PANEL = IM_COL32(244, 234, 210, 255), PANEL2 = IM_COL32(236, 224, 196, 255), HOVER = IM_COL32(226, 204, 160, 255);
    static int hover = -1, last_state = -1;                   /* (the childhood or profession pointed at last: it stays shown) */
    if (c.state != last_state) { hover = -1; last_state = c.state; }
    char b[320];

    /* the page, the whole screen */
    P.rect(0, 0, 320, 200, IM_COL32(236, 222, 188, 255));
    P.rect(1.5f, 1.5f, 318.5f, 198.5f, IM_COL32(242, 230, 200, 255), 2);
    P.frame(1.5f, 1.5f, 318.5f, 198.5f, IM_COL32(120, 80, 36, 150), 2);

    /* the header: who, and where in the making */
    std::string name = ns_gtext(c.name), nick = ns_gtext(c.nick);
    P.text(f, NsDraw::FS_TITLE, 6, 2.5f, ink, name.empty() ? "A new character" : name, -1, 170);
    if (c.age > 0) snprintf(b, sizeof b, "%s, called %s, age %d", c.female ? "A woman" : "A man", nick.c_str(), c.age);
    else snprintf(b, sizeof b, "%s, called %s", c.female ? "A woman" : "A man", nick.c_str());   /* (no age before the childhood) */
    P.text(fo, NsDraw::FS_BODY, 6, 12, soft, b);
    static const char *STEP[5] = { "Name", "Childhood", "Attributes", "Profession", "Skills" };
    for (int i = 0; i < 5; i++) {
        float x0 = 172 + i * 28.8f, x1 = x0 + 28;
        bool cur = i == c.state, done = i < c.state;
        P.tab(x0, 3, x1, 12.5f, cur, done);
        snprintf(b, sizeof b, "%d  %s", i + 1, STEP[i]);
        P.text(fo, NsDraw::FS_SMALL, x0 + (28 - P.width(fo, NsDraw::FS_SMALL, b)) * 0.5f, 5.0f, cur ? NsDraw::LIGHT : done ? ink : dim, b);
    }

    const float Y0 = 18, Y1 = 196;
    /* the two panels: their split depends on the step */
    float LX0 = 5, LX1 = c.state == 3 ? 162 : c.state == 4 ? 206 : 150, RX0 = LX1 + 3, RX1 = 315;
    P.rect(LX0, Y0, LX1, Y1, PANEL, 1.5f);
    P.rect(RX0, Y0, RX1, Y1, PANEL2, 1.5f);
    float y = Y0 + 2.5f;
    auto title = [&](const std::string &t) { P.text(f, NsDraw::FS_HEAD, LX0 + 3, y, ink, t); y += 9.5f; };
    auto note = [&](const std::string &t) { y += P.para(fo, NsDraw::FS_SMALL, LX0 + 3, y, LX1 - LX0 - 6, soft, t) + 2.5f; };
    int new_hover = -1;

    if (c.state == 0) {
        title("Name and gender");
        note("Type a name or pick a random one, then go on to the childhood.");
        const char *lab[2] = { "Name", "Short name (the one the cards use)" };
        for (int k = 0; k < 2; k++) {
            P.text(fo, NsDraw::FS_SMALL, LX0 + 3, y, soft, lab[k]);
            y += 6;
            bool typing = c.typing == k + 1;
            P.rect(LX0 + 3, y, LX1 - 3, y + 11, typing ? IM_COL32(255, 250, 236, 255) : IM_COL32(236, 224, 196, 255), 1.2f);
            P.frame(LX0 + 3, y, LX1 - 3, y + 11, IM_COL32(120, 80, 36, typing ? 200 : 90), 1.2f);
            std::string v = ns_gtext(typing ? c.typed : k ? c.nick : c.name);
            P.text(f, NsDraw::FS_NAME, LX0 + 5, y + 2.2f, ink, v, -1, LX1 - 5);
            if (typing && ((plat_ticks_ms() - c.caret_t) / 500) % 2 == 0) { float cx = LX0 + 5.3f + P.width(f, NsDraw::FS_NAME, v); P.rect(cx, y + 2.5f, cx + 0.6f, y + 8.8f, ink); }
            if (!typing && P.hit(LX0 + 3, y, LX1 - 3, y + 11)) ns_push(CG_TYPE, k + 1, 0, 0);
            y += 14;
        }
        if (c.typing) note("Enter keeps it, Esc cancels.");
        float mid = (LX0 + LX1) * 0.5f;
        if (P.button(LX0 + 3, y, mid - 1.5f, y + 11, "Random name", BROWN, NsDraw::FS_BODY)) ns_push(CG_KEY, 3, 0, 0);
        if (P.button(mid + 1.5f, y, LX1 - 3, y + 11, c.female ? "Be a man" : "Be a woman", BROWN, NsDraw::FS_BODY)) ns_push(CG_KEY, 2, 0, 0);
        y += 15;
        if (P.button(LX0 + 3, y, LX1 - 3, y + 13, "On to the childhood", GREEN, NsDraw::FS_HEADBTN)) ns_push(CG_KEY, 1, 0, 0);
        if (P.button(LX0 + 3, Y1 - 15, LX1 - 3, Y1 - 3, "Cancel and return to the party", RED, NsDraw::FS_BODY)) ns_push(CG_KEY, 6, 0, 0);
    } else if (c.state == 1) {
        title("Childhood");
        note("The family decides the starting attributes and skills, and which professions are open at first. Point at one to see what it brings (on the right); click to choose it.");
        y += 1;
        for (int i = 0; i < c.nlist; i++) {
            float ry = y + i * 12;
            int p = c.list[i];
            bool h = P.over(LX0 + 1, ry, LX1 - 1, ry + 11.5f);
            if (h) new_hover = p;
            if (h || hover == p) P.rect(LX0 + 1.5f, ry, LX1 - 1.5f, ry + 11.5f, HOVER, 1.0f);
            P.text(f, NsDraw::FS_NAME, LX0 + 4, ry + 2.5f, ink, ns_gtext(c.child_name[p]), -1, LX1 - 3);
            if (P.hit(LX0 + 1, ry, LX1 - 1, ry + 11.5f)) ns_push(CG_KEY, i + 1, 0, 0);
        }
    } else if (c.state == 3) {
        title("Profession");
        note("Five years each. Those in dark letters are open now: click one to take it. Point at any to see what it needs and brings.");
        /* every profession, by kind, in three columns: the open ones dark and clickable */
        static const int COLS[3][3] = { { 0, 1, 5 }, { 2, 3, -1 }, { 4, 6, -1 } };
        float cw = (LX1 - LX0 - 6) / 3.0f, y0 = y;
        for (int col = 0; col < 3; col++) {
            float x = LX0 + 3 + col * cw, yy = y0;
            for (int gi = 0; gi < 3; gi++) {
                int g = COLS[col][gi];
                if (g < 0) continue;
                ui_draw_career_icon(P.dl, NsDraw::w(x + 4.2f, yy + 3.6f), 7.6f * P.k, g);   /* (the kind's picture) */
                P.text(f, NsDraw::FS_BODY, x + 9.5f, yy + 0.9f, soft, GROUPS[g].name);
                P.rect(x + 9.5f, yy + 7.4f, x + cw - 3, yy + 7.7f, IM_COL32(120, 80, 36, 90));
                yy += 9.5f;
                for (int k = 0; k < GROUPS[g].n; k++) {
                    int p = GROUPS[g].first + k, li = -1;
                    for (int i = 0; i < c.nlist; i++) if (c.list[i] == p) li = i;
                    bool h = P.over(x, yy, x + cw - 2, yy + 8);
                    if (h) new_hover = p;
                    if (h || hover == p) P.rect(x, yy, x + cw - 2, yy + 8, li >= 0 ? HOVER : IM_COL32(232, 222, 200, 255), 1.0f);
                    std::string nm = ns_gtext(c.prof_name[p]);
                    float tx = x + 1.5f;
                    if (c.taken[p]) { P.dl->AddCircleFilled(NsDraw::w(x + 2.2f, yy + 4.2f), 0.9f * P.k, li >= 0 ? NsDraw::GREEN : dim); tx += 2.6f; }
                    P.text(li >= 0 ? f : fo, li >= 0 ? NsDraw::FS_BODY + 0.3f : NsDraw::FS_BODY - 0.2f, tx, yy + 1.4f, li >= 0 ? ink : IM_COL32(150, 132, 104, 255), nm, -1, x + cw - 3);
                    if (li >= 0 && P.hit(x, yy, x + cw - 2, yy + 8)) ns_push(CG_KEY, li + 1, 0, 0);
                    yy += 8;
                }
                yy += 2.5f;
            }
        }
        float ly = Y1 - 9;
        P.dl->AddCircleFilled(NsDraw::w(LX0 + 5, ly + 3.2f), 0.9f * P.k, NsDraw::GREEN);
        P.text(fo, NsDraw::FS_SMALL, LX0 + 7.5f, ly + 0.5f, dim, "taken before (can be taken again for five more years)");
        if (!c.nlist) P.text(fo, NsDraw::FS_SMALL, LX0 + 3, ly - 7, NsDraw::RED, "No profession is open to this character now.");
    } else if (c.state == 2) {
        title("Attributes");
        snprintf(b, sizeof b, "%d of %d EPs left. Use + and - on a row (- only takes back this step's points). A point costs 1 EP, 2 at 29+, 3 at 39+; an attribute can reach 40 at most.", c.ep, c.ep_step);
        note(b);
        y += 1;
        for (int i = 0; i < 6; i++) {
            float ry = y + i * 13, rh = 12;
            int v = c.attr[i], sp = c.attr_spent[i];
            bool sel = c.row == i, h = P.over(LX0 + 1, ry, LX1 - 1, ry + rh);
            if (sel || h) P.rect(LX0 + 1.5f, ry, LX1 - 1.5f, ry + rh, sel ? HOVER : IM_COL32(236, 222, 190, 255), 1.0f);
            ui_attr_icon(P.dl, NsDraw::w(LX0 + 7, ry + rh * 0.5f), 6.8f * P.k, i);
            P.text(f, NsDraw::FS_NAME, LX0 + 14, ry + 3, ink, gm_attr_name(i));
            snprintf(b, sizeof b, "%d", v);
            float vx = LX0 + 70;
            P.text(f, NsDraw::FS_NAME, 0, ry + 3, ink, b, vx);
            if (sp) { snprintf(b, sizeof b, "+%d", sp); P.text(fo, NsDraw::FS_SMALL, vx + 1.5f, ry + 4, NsDraw::GREEN, b); }
            snprintf(b, sizeof b, "%d EP a point", attr_cost(v));
            P.text(fo, NsDraw::FS_SMALL, vx + 13, ry + 4, dim, b);
            float bw = 9, bx = LX1 - 3;
            bool can_plus = c.ep >= attr_cost(v) && sp < 40 && v < 40;
            if (can_plus && P.button(bx - bw, ry + 1.5f, bx, ry + rh - 1.5f, "+", GREEN, NsDraw::FS_HEAD)) ns_push(CG_ROWPLUS, i, 0, 0);
            if (sp && P.button(bx - 2 * bw - 1.5f, ry + 1.5f, bx - bw - 1.5f, ry + rh - 1.5f, "-", BROWN, NsDraw::FS_HEAD)) ns_push(CG_ROWMINUS, i, 0, 0);
        }
        if (P.button(LX0 + 3, Y1 - 15, LX1 - 3, Y1 - 3, "On to the profession", GREEN, NsDraw::FS_HEADBTN)) ns_push(CG_KEY, 1, 0, 0);
    } else if (c.state == 4) {
        title("Skills");
        snprintf(b, sizeof b, "%d of %d EPs left. A point costs 1 EP, 2 at 49+, 3 at 79+. Room: how many more points this profession still lets you add to the skill.", c.ep, c.ep_step);
        note(b);
        const CgProf &ad = c.adult[c.prof >= 0 && c.prof < 37 ? c.prof : 0];
        float cw = (LX1 - LX0 - 6) / 2.0f, rh = 9.4f;
        for (int col = 0; col < 2; col++) {                   /* the heads of the columns */
            float x = LX0 + 3 + col * cw;
            P.text(fo, NsDraw::FS_SMALL, x + 10, y, dim, "skill");
            P.text(fo, NsDraw::FS_SMALL, 0, y, dim, "now", x + 52);
            P.text(fo, NsDraw::FS_SMALL, 0, y, dim, "room", x + 72);
        }
        y += 6.5f;
        for (int i = 0; i < 19; i++) {
            int col = i < 10 ? 0 : 1, r = i < 10 ? i : i - 10;
            float x = LX0 + 3 + col * cw, ry = y + r * rh;
            int row = skill_row(i), v = c.skill[i], sp = c.skill_spent[i], room = ad.maxsk[i] - sp;
            bool sel = c.row == row, h = P.over(x, ry, x + cw - 2, ry + rh);
            if (sel || h) P.rect(x, ry, x + cw - 2, ry + rh, sel ? HOVER : IM_COL32(236, 222, 190, 255), 1.0f);
            ui_skill_icon(P.dl, NsDraw::w(x + 4, ry + rh * 0.5f), 6.4f * P.k, i);
            P.text(f, NsDraw::FS_BODY + 0.2f, x + 10, ry + 2.2f, ink, gm_skill_short(i), -1, x + 42);
            snprintf(b, sizeof b, "%d", v);
            P.text(f, NsDraw::FS_BODY + 0.2f, 0, ry + 2.2f, ink, b, x + 52);
            if (sp) { snprintf(b, sizeof b, "+%d", sp); P.text(fo, NsDraw::FS_SMALL - 0.3f, x + 53, ry + 2.8f, NsDraw::GREEN, b); }
            snprintf(b, sizeof b, "%d", room < 0 ? 0 : room);
            P.text(fo, NsDraw::FS_BODY, x + 72 - P.width(fo, NsDraw::FS_BODY, b) * 0.5f - 3, ry + 2.4f, room > 0 ? soft : IM_COL32(170, 150, 120, 255), room > 0 ? b : "-");
            float bw = 6.5f, bx = x + cw - 3;
            bool can_plus = c.ep >= skill_cost(v) && room > 0 && v < 89;
            if (can_plus && P.button(bx - bw, ry + 1, bx, ry + rh - 1, "+", GREEN, NsDraw::FS_BODY)) ns_push(CG_ROWPLUS, row, 0, 0);
            if (sp && P.button(bx - 2 * bw - 1, ry + 1, bx - bw - 1, ry + rh - 1, "-", BROWN, NsDraw::FS_BODY)) ns_push(CG_ROWMINUS, row, 0, 0);
            if (h) {
                snprintf(b, sizeof b, "%s %d: a point costs %d EP now; this profession allows %d more.", gm_skill_short(i), v, skill_cost(v), room < 0 ? 0 : room);
                P.text(fo, NsDraw::FS_SMALL, LX0 + 3, y + 10 * rh + 2, soft, b, -1, LX1 - 3);
            }
        }
        float mid = (LX0 + LX1) * 0.5f;
        if (P.button(LX0 + 3, Y1 - 29, LX1 - 3, Y1 - 17, "Another profession (five more years)", BROWN, NsDraw::FS_BODY)) ns_push(CG_KEY, 1, 0, 0);
        if (P.button(LX0 + 3, Y1 - 15, mid - 1.5f, Y1 - 3, "Begin adventuring", GREEN, NsDraw::FS_HEADBTN)) ns_push(CG_KEY, 2, 0, 0);
        if (P.button(mid + 1.5f, Y1 - 15, LX1 - 3, Y1 - 3, "Start over", RED, NsDraw::FS_BODY)) ns_push(CG_KEY, 3, 0, 0);
    }
    if (new_hover >= 0) hover = new_hover;

    /* ---- the right side ---- */
    Preview pv = preview(c, (c.state == 1 || c.state == 3) ? hover : -1);
    float ry = Y0 + 2.5f, RW = RX1 - RX0 - 6;
    bool choosing = (c.state == 1 || c.state == 3) && hover >= 0;
    bool offered = false;
    if (c.state == 3 && hover >= 0) for (int i = 0; i < c.nlist; i++) if (c.list[i] == hover) offered = true;
    if (choosing) {
        std::string head = ns_gtext(c.state == 1 ? c.child_name[hover] : c.prof_name[hover]);
        P.text(f, NsDraw::FS_HEAD, RX0 + 3, ry, ink, head, -1, RX1 - 30);
        if (c.state == 3) {
            const char *tag = offered ? "open now" : "not open";
            P.tag(RX1 - 3 - P.tag_width(tag), ry - 1.5f, 10, tag, offered ? NsDraw::GREEN : NsDraw::RED);
        }
        ry += 9.5f;
        snprintf(b, sizeof b, c.state == 1 ? "%d EPs for the attributes, then age %d." : "%d EPs for the skills, then age %d.", pv.ep, c.age + (c.state == 1 ? 15 : 5));
        P.text(fo, NsDraw::FS_BODY, RX0 + 3, ry, soft, b);
        ry += 7;
        if (c.state == 3 && c.age + 5 > 30) { P.text(fo, NsDraw::FS_SMALL, RX0 + 3, ry, NsDraw::RED, "Over 30, aging lowers attributes (shown below)."); ry += 6.5f; }
        if (c.state == 3) {                                   /* what it needs, what this character lacks, what it leads to */
            CgCtx x = ctx_of(c);
            ry += P.para(fo, NsDraw::FS_SMALL, RX0 + 3, ry, RW, ink, std::string("Needs: ") + cg_prof_needs(hover)) + 1.5f;
            if (!offered) {
                std::string why;
                if (cg_prof_rule(hover, x)) why = "This character qualifies, but the game lists at most 15 professions and this one comes after them.";
                else why = cg_prof_why(hover, x);
                ry += P.para(fo, NsDraw::FS_SMALL, RX0 + 3, ry, RW, NsDraw::RED, why) + 1.5f;
            }
            std::vector<int> lt = cg_prof_leads_to(hover);
            if (!lt.empty()) ry += P.para(fo, NsDraw::FS_SMALL, RX0 + 3, ry, RW, soft, "Leads to: " + join_profs(c, lt) + ".") + 1.5f;
        } else {                                              /* a childhood: the professions it opens at first */
            CgCtx x = ctx_of(c); x.age = 15; x.child = hover;
            for (int k = 0; k < 6; k++) x.attr[k] = (signed char)pv.attr[k];
            for (int k = 0; k < 19; k++) x.skill[k] = (signed char)(c.skill[k] + pv.gain[k]);
            ry += P.para(fo, NsDraw::FS_SMALL, RX0 + 3, ry, RW, ink, "Professions open at first: " + join_profs(c, cg_prof_offered(x)) + ".") + 1.5f;
        }
        ry += 1.5f;
    } else {
        P.text(f, NsDraw::FS_HEAD, RX0 + 3, ry, ink, "The character so far");
        ry += 9.5f;
        if (c.state == 1 || c.state == 3) { P.text(fo, NsDraw::FS_SMALL, RX0 + 3, ry, dim, c.state == 1 ? "Point at a childhood to see what it brings." : "Point at a profession to see what it needs and brings."); ry += 7; }
    }

    /* the attributes (not on the attribute step: they are on the left), with what the row pointed at changes */
    if (c.state != 2) {
        bool one_col = RW < 120;
        int ncol = one_col ? 1 : 3;
        float cw = RW / ncol, rh = 7.4f;
        for (int i = 0; i < 7; i++) {
            float x = RX0 + 3 + (i % ncol) * cw, yy = ry + (i / ncol) * rh;
            ui_attr_icon(P.dl, NsDraw::w(x + 2.8f, yy + 3), 5.0f * P.k, i);
            P.text(fo, NsDraw::FS_SMALL, x + 7.5f, yy + 0.6f, soft, i == 6 ? "Div. Favor" : gm_attr_name(i));
            int now = c.attr[i], after = i < 6 ? pv.attr[i] : now;
            snprintf(b, sizeof b, "%d", now);
            float vx = x + (one_col ? 52 : cw * 0.80f);
            P.text(f, NsDraw::FS_BODY, 0, yy, ink, b, vx);
            if (choosing && i < 6 && after != now) {
                snprintf(b, sizeof b, "%s%d", after > now ? "+" : "", after - now);
                P.text(fo, NsDraw::FS_SMALL, vx + 1.2f, yy + 0.6f, after > now ? NsDraw::GREEN : NsDraw::RED, b);
            }
        }
        ry += ((7 + ncol - 1) / ncol) * rh + 2.5f;
    }

    /* the skills (not on the skill step), with the gain and the room the row pointed at gives */
    if (c.state != 4) {
        bool gains = choosing, room = choosing && c.state == 3 && pv.adult;
        float cw = RW / 2, rh = 7.0f;
        /* the columns' right edges: now, gain, room (narrower with all three) */
        float xn = room ? cw * 0.62f : gains ? cw * 0.70f : cw * 0.80f, xg = room ? cw * 0.79f : cw * 0.90f, xr = cw * 0.95f, xname = xn - 6;
        for (int col = 0; col < 2; col++) {
            float x = RX0 + 3 + col * cw;
            P.text(fo, NsDraw::FS_SMALL - 0.4f, x + 7, ry, dim, "skill");
            P.text(fo, NsDraw::FS_SMALL - 0.4f, 0, ry, dim, "now", x + xn);
            if (gains) P.text(fo, NsDraw::FS_SMALL - 0.4f, 0, ry, dim, c.state == 1 ? "gains" : "teaches", x + xg);
            if (room) P.text(fo, NsDraw::FS_SMALL - 0.4f, 0, ry, dim, "room", x + xr);
        }
        ry += 6;
        for (int i = 0; i < 19; i++) {
            int col = i < 10 ? 0 : 1, r = i < 10 ? i : i - 10;
            float x = RX0 + 3 + col * cw, yy = ry + r * rh;
            ui_skill_icon(P.dl, NsDraw::w(x + 2.8f, yy + 3), 5.0f * P.k, i);
            P.text(fo, NsDraw::FS_SMALL, x + 7.5f, yy + 0.6f, soft, gm_skill_short(i), -1, x + xname);
            snprintf(b, sizeof b, "%d", c.skill[i]);
            P.text(f, NsDraw::FS_BODY, 0, yy, ink, b, x + xn);
            if (gains) {
                int g = pv.gain[i];
                snprintf(b, sizeof b, "+%d", g);
                P.text(fo, NsDraw::FS_SMALL, 0, yy + 0.6f, g > 0 ? NsDraw::GREEN : IM_COL32(170, 150, 120, 255), g > 0 ? b : "-", x + xg);
            }
            if (room) {
                int rm = pv.room[i] < 0 ? 0 : pv.room[i];
                snprintf(b, sizeof b, "%d", rm);
                P.text(fo, NsDraw::FS_SMALL, 0, yy + 0.6f, rm > 0 ? soft : IM_COL32(170, 150, 120, 255), rm > 0 ? b : "-", x + xr);
            }
        }
        ry += 10 * rh + 2;
        if (room) { ry += P.para(fo, NsDraw::FS_SMALL - 0.4f, RX0 + 3, ry, RW, dim, "teaches: the points the five years give; room: how many more points skill EPs can add afterwards.") + 1.5f; }
    }

    /* the past: professions, virtues, saints */
    if (!choosing) {
        std::vector<int> past;
        for (int p = 0; p < 37; p++) if (c.taken[p]) past.push_back(p);
        if (!past.empty() && ry < Y1 - 8) ry += P.para(fo, NsDraw::FS_SMALL, RX0 + 3, ry, RW, soft, "Has been: " + join_profs(c, past) + ".") + 1;
    }
    if (c.nvirtue || c.nsaint) {
        std::string v;
        for (int i = 0; i < c.nvirtue; i++) { if (i) v += ", "; v += ns_gtext(c.virtue[i]); }
        if (!v.empty() && ry < Y1 - 8) ry += P.para(fo, NsDraw::FS_SMALL, RX0 + 3, ry, RW, soft, "Virtues: " + v) + 1;
        std::string s;
        for (int i = 0; i < c.nsaint; i++) { int k = c.saint[i]; if (k >= 0 && k < (int)gamedata.saints.size()) { if (!s.empty()) s += ", "; s += gamedata.saints[k].name; } }
        if (!s.empty() && ry < Y1 - 8) P.para(fo, NsDraw::FS_SMALL, RX0 + 3, ry, RW, soft, "Saints known: " + s);
    }
}
