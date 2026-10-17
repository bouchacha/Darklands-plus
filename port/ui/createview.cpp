// createview.cpp - making a character, drawn by Darklands+ (mods/createview.c runs it). Left: the
// step (name and gender; the childhood; the attribute EPs; the profession; the skill EPs). Right:
// the character as it stands, and what the childhood or profession under the pointer would bring
// (the attributes after it, with the aging after thirty, and each skill's gain and room), its
// virtues and the saints it knows.
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string>
#include "nsdraw.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../mods/createview.h"
u32 plat_ticks_ms(void);
}
void ui_attr_icon(ImDrawList *dl, ImVec2 c, float s, int k);
void ui_skill_icon(ImDrawList *dl, ImVec2 c, float s, int k);

static const char *ATTR[7] = { "Endurance", "Strength", "Agility", "Perception", "Intellect", "Charisma", "Divine Favor" };
static const char *SKILL[19] = { "Edged", "Impact", "Flails", "Polearms", "Thrown", "Bows", "Missile devices",
    "Alchemy", "Religion", "Virtue", "Speak Common", "Speak Latin", "Read & Write", "Healing", "Artifice", "Stealth",
    "Streetwise", "Riding", "Woodwise" };
static int skill_row(int s) { return s < 7 ? s : s < 13 ? s + 1 : s + 2; }   /* the game's row (gaps 7, 14) */
static int attr_cost(int v) { return v >= 39 ? 3 : v >= 29 ? 2 : 1; }
static int skill_cost(int v) { return v >= 79 ? 3 : v >= 49 ? 2 : 1; }

/* the game's preview (s037 18c8:1738): the profession under the pointer, or the one being spent */
struct Preview { bool on; int prn; int ep; int attr[6]; int gain[19], room[19]; bool adult; };
static Preview preview(const CreateModel &c, int hover)
{
    Preview p = {};
    int prn = (c.state == 1 || c.state == 3) ? (hover >= 0 && hover < c.nlist ? c.list[hover] : -1) : c.state == 2 ? c.child : c.prof;
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

extern "C" void createview_draw(void)
{
    const CreateModel &c = create_model;
    if (!c.open || ns_screen != NS_CREATE || ns_hidden()) return;
    NsDraw P;
    if (!P.begin()) return;
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM;
    const ImU32 GREEN = IM_COL32(40, 92, 34, 235), BROWN = IM_COL32(120, 84, 40, 235), RED = IM_COL32(112, 40, 22, 235);
    P.page();
    static int hover = -1, last_state = -1; static unsigned hover_seq;
    if (c.state != last_state) { hover = -1; last_state = c.state; }

    /* the header */
    std::string name = ns_gtext(c.name), nick = ns_gtext(c.nick);
    P.text(f, NsDraw::FS_TITLE, 65, 2.5f, ink, name.empty() ? "A new character" : name);
    char b[256];
    snprintf(b, sizeof b, "%s, called %s  -  age %d", c.female ? "a woman" : "a man", nick.c_str(), c.age);
    P.text(fo, NsDraw::FS_SMALL, 65, 12, soft, b);
    static const char *STEP[5] = { "Name and gender", "Childhood", "Attributes", "Profession", "Skills" };
    for (int i = 0; i < 5; i++) {                             /* where in the making */
        float x = 196 + i * 24.3f;
        bool cur = i == c.state || (c.state == 4 && i == 3 && false);
        P.rect(x, 3, x + 23, 10, cur ? IM_COL32(112, 40, 22, 240) : IM_COL32(222, 204, 168, 255), 1.5f);
        P.text(fo, 3.9f, x + (23 - P.width(fo, 3.9f, STEP[i])) * 0.5f, 4.4f, cur ? NsDraw::LIGHT : dim, STEP[i]);
    }

    const float LX0 = 63, LX1 = 182, RX0 = 185, RX1 = 317, Y0 = 19;
    P.rect(LX0, Y0, LX1, 196, IM_COL32(244, 234, 210, 255), 1.5f);
    float y = Y0 + 2;
    auto title = [&](const std::string &t) { y += P.para(f, NsDraw::FS_HEAD, LX0 + 2.5f, y, LX1 - LX0 - 5, ink, t) + 1.5f; };
    auto note = [&](const std::string &t) { y += P.para(fo, NsDraw::FS_SMALL, LX0 + 2.5f, y, LX1 - LX0 - 5, soft, t) + 2; };
    int new_hover = -1;

    if (c.state == 0) {
        title("Name and gender");
        note("Type a name, or have one drawn; then the childhood.");
        const char *lab[2] = { "Name", "Short name" };
        for (int k = 0; k < 2; k++) {
            P.text(fo, NsDraw::FS_SMALL, LX0 + 2.5f, y, soft, lab[k]);
            y += 6;
            bool typing = c.typing == k + 1;
            P.rect(LX0 + 2, y, LX1 - 2, y + 9, typing ? IM_COL32(255, 250, 236, 255) : IM_COL32(236, 224, 196, 255), 1.2f);
            P.frame(LX0 + 2, y, LX1 - 2, y + 9, IM_COL32(120, 80, 36, typing ? 200 : 90), 1.2f);
            std::string v = ns_gtext(typing ? c.typed : k ? c.nick : c.name);
            P.text(f, NsDraw::FS_NAME, LX0 + 4, y + 1.2f, ink, v, -1, LX1 - 4);
            if (typing && ((plat_ticks_ms() - c.caret_t) / 500) % 2 == 0) { float cx = LX0 + 4.3f + P.width(f, NsDraw::FS_NAME, v); P.rect(cx, y + 1.5f, cx + 0.6f, y + 7.8f, ink); }
            if (!typing && P.hit(LX0 + 2, y, LX1 - 2, y + 9)) ns_push(CG_TYPE, k + 1, 0, 0);
            y += 11;
        }
        if (c.typing) note("Enter keeps it, Esc does not.");
        if (P.button(LX0 + 2, y, LX0 + 56, y + 9, "Draw a name", BROWN)) ns_push(CG_KEY, 3, 0, 0);
        if (P.button(LX0 + 59, y, LX1 - 2, y + 9, c.female ? "Be a man" : "Be a woman", BROWN)) ns_push(CG_KEY, 2, 0, 0);
        y += 13;
        if (P.button(LX0 + 2, y, LX1 - 2, y + 11, "On to the childhood", GREEN, NsDraw::FS_BODY)) ns_push(CG_KEY, 1, 0, 0);
        y += 14;
        if (P.button(LX0 + 2, 184, LX1 - 2, 194, "Back to the party (no character)", RED)) ns_push(CG_KEY, 6, 0, 0);
    } else if (c.state == 1 || c.state == 3) {
        title(c.state == 1 ? "Childhood" : "Profession");
        note(c.state == 1 ? "The family decides the first attributes and skills (the right side shows the one under the pointer)."
                          : "Five years in a profession (the right side: what the one under the pointer brings). Some open only after others, or need certain attributes.");
        for (int i = 0; i < c.nlist; i++) {
            float ry = y + i * 9.2f;
            if (ry + 9 > 194) break;
            int p = c.list[i];
            bool h = P.over(LX0 + 1, ry, LX1 - 1, ry + 9);
            if (h) new_hover = i;
            if (h || hover == i) P.rect(LX0 + 1, ry, LX1 - 1, ry + 9, IM_COL32(226, 204, 160, 255), 1.0f);
            std::string nm = ns_gtext(c.state == 1 ? c.child_name[p] : c.prof_name[p]);
            P.text(f, NsDraw::FS_NAME, LX0 + 3, ry + 1.2f, ink, nm, -1, LX1 - 18);
            if (c.state == 3 && c.taken[p]) P.text(fo, 3.9f, 0, ry + 2.4f, dim, "again", LX1 - 3);
            if (P.hit(LX0 + 1, ry, LX1 - 1, ry + 9)) ns_push(CG_KEY, i + 1, 0, 0);
        }
        if (c.state == 3 && !c.nlist) note("No profession is open to this character now.");
    } else if (c.state == 2 || c.state == 4) {
        bool at = c.state == 2;
        title(at ? "Attributes" : "Skills");
        snprintf(b, sizeof b, "%d of %d EPs left. + and - on a row (- takes back what this step gave). Each point costs 1, 2 or 3 EPs as the value grows.", c.ep, c.ep_step);
        note(b);
        int n = at ? 6 : 19;
        float rh = at ? 9.0f : 6.6f;
        for (int i = 0; i < n; i++) {
            float ry = y + i * rh + (!at && i >= 7 ? 1.5f : 0) + (!at && i >= 13 ? 1.5f : 0);
            int row = at ? i : skill_row(i);
            int v = at ? c.attr[i] : c.skill[i], sp = at ? c.attr_spent[i] : c.skill_spent[i];
            bool sel = c.row == row, h = P.over(LX0 + 1, ry, LX1 - 1, ry + rh);
            if (sel || h) P.rect(LX0 + 1, ry, LX1 - 1, ry + rh, sel ? IM_COL32(226, 204, 160, 255) : IM_COL32(236, 222, 190, 255), 0.8f);
            ImVec2 ic = NsDraw::w(LX0 + 4.5f, ry + rh * 0.5f);
            if (at) ui_attr_icon(P.dl, ic, rh * 0.7f * P.k, i); else ui_skill_icon(P.dl, ic, rh * 0.75f * P.k, i);
            float fs = at ? NsDraw::FS_NAME : 4.6f;
            P.text(at ? f : fo, fs, LX0 + 10, ry + (at ? 1.2f : 0.7f), ink, at ? ATTR[i] : SKILL[i]);
            snprintf(b, sizeof b, "%d", v);
            float vx = LX1 - 26;
            P.text(f, fs, 0, ry + (at ? 1.2f : 0.7f), ink, b, vx);
            if (sp) { snprintf(b, sizeof b, "+%d", sp); P.text(fo, 3.9f, vx + 1, ry + (at ? 2.4f : 1.4f), NsDraw::GREEN, b); }
            float bw = at ? 6.5f : 5.4f, bx = LX1 - 2;
            int room = at ? 99 : 0;
            if (!at) room = c.adult[c.prof >= 0 && c.prof < 37 ? c.prof : 0].maxsk[i] - sp;
            bool can_plus = c.ep >= (at ? attr_cost(v) : skill_cost(v)) && (at ? (sp < 40 && v < 40) : (room > 0 && v < 89));
            if (can_plus && P.button(bx - bw, ry + 0.4f, bx, ry + rh - 0.4f, "+", GREEN, at ? NsDraw::FS_BODY : 4.4f)) ns_push(CG_ROWPLUS, row, 0, 0);
            if (sp && P.button(bx - 2 * bw - 1, ry + 0.4f, bx - bw - 1, ry + rh - 0.4f, "-", BROWN, at ? NsDraw::FS_BODY : 4.4f)) ns_push(CG_ROWMINUS, row, 0, 0);
            if (h && !at) { snprintf(b, sizeof b, "%s: %d; a point costs %d EP; room this profession: %d", SKILL[i], v, skill_cost(v), room < 0 ? 0 : room); ImGui::SetTooltip("%s", b); }
            if (h && at) { snprintf(b, sizeof b, "%s: %d; a point costs %d EP (at most 40 here)", ATTR[i], v, attr_cost(v)); ImGui::SetTooltip("%s", b); }
        }
        if (at) { if (P.button(LX0 + 2, 182, LX1 - 2, 194, "On to the profession", GREEN, NsDraw::FS_BODY)) ns_push(CG_KEY, 1, 0, 0); }
        else {
            if (P.button(LX0 + 2, 172, LX1 - 2, 182, "Another profession (5 more years)", BROWN)) ns_push(CG_KEY, 1, 0, 0);
            if (P.button(LX0 + 2, 184, LX0 + 76, 194, "Begin adventuring", GREEN)) ns_push(CG_KEY, 2, 0, 0);
            if (P.button(LX0 + 79, 184, LX1 - 2, 194, "Start over", RED)) ns_push(CG_KEY, 3, 0, 0);
        }
    }
    if (new_hover >= 0) { hover = new_hover; hover_seq = c.seq; }

    /* the right side: the character, and what the row under the pointer would bring */
    Preview pv = preview(c, hover);
    P.rect(RX0, Y0, RX1, 196, IM_COL32(236, 224, 196, 255), 1.5f);
    float ry = Y0 + 2;
    bool choosing = (c.state == 1 || c.state == 3) && pv.on && hover >= 0;
    std::string head = choosing ? ns_gtext(c.state == 1 ? c.child_name[pv.prn] : c.prof_name[pv.prn]) : std::string("As it stands");
    P.text(f, NsDraw::FS_HEAD, RX0 + 2.5f, ry, ink, head, -1, RX1 - 3);
    ry += 8;
    if (choosing) {
        snprintf(b, sizeof b, c.state == 1 ? "%d EPs for the attributes; then age %d" : "%d EPs for the skills; then age %d", pv.ep, c.age + (c.state == 1 ? 15 : 5));
        P.text(fo, NsDraw::FS_SMALL, RX0 + 2.5f, ry, soft, b);
        ry += 6.5f;
        if (c.state == 3 && c.age + 5 >= 30) { P.text(fo, NsDraw::FS_SMALL, RX0 + 2.5f, ry, NsDraw::RED, "Past thirty, the years cost attributes."); ry += 6.5f; }
    } else if (c.state == 1 || c.state == 3) { P.text(fo, NsDraw::FS_SMALL, RX0 + 2.5f, ry, dim, "Point at a row to see what it brings."); ry += 6.5f; }
    /* attributes, two columns */
    for (int i = 0; i < 7; i++) {
        float x = RX0 + 2.5f + (i % 2) * 65, yy = ry + (i / 2) * 7;
        ui_attr_icon(P.dl, NsDraw::w(x + 2.4f, yy + 2.7f), 4.6f * P.k, i);
        P.text(fo, 4.4f, x + 6.5f, yy, soft, ATTR[i]);
        int now = c.attr[i], after = i < 6 ? pv.attr[i] : now;
        snprintf(b, sizeof b, "%d", now);
        P.text(f, 4.8f, 0, yy - 0.3f, ink, b, x + 52);
        if (choosing && i < 6 && after != now) {
            snprintf(b, sizeof b, "%s%d", after > now ? "+" : "", after - now);
            P.text(fo, 4.2f, x + 53.5f, yy + 0.2f, after > now ? NsDraw::GREEN : NsDraw::RED, b);
        }
    }
    ry += 4 * 7 + 2;
    /* skills: now, and the gain : room the profession gives */
    bool show_gain = c.state == 1 || c.state == 3 || c.state == 4;
    P.text(fo, 4.0f, RX0 + 2.5f, ry, dim, show_gain ? (c.state == 1 ? "skill  now  (the childhood adds)" : "skill  now  (gain : room this profession)") : "skill  now");
    ry += 5.5f;
    for (int i = 0; i < 19; i++) {
        float x = RX0 + 2.5f + (i < 10 ? 0 : 65), yy = ry + (i < 10 ? i : i - 10) * 6.6f;
        ui_skill_icon(P.dl, NsDraw::w(x + 2.4f, yy + 2.6f), 4.4f * P.k, i);
        P.text(fo, 4.2f, x + 6.5f, yy, soft, SKILL[i]);
        snprintf(b, sizeof b, "%d", c.skill[i]);
        P.text(f, 4.6f, 0, yy - 0.3f, ink, b, x + 44);
        if (show_gain && (pv.on || c.state == 4)) {
            if (c.state == 1 || !pv.adult) snprintf(b, sizeof b, "+%d", pv.gain[i]);
            else snprintf(b, sizeof b, "%d:%d", pv.gain[i], pv.room[i] < 0 ? 0 : pv.room[i]);
            P.text(fo, 4.0f, x + 45.5f, yy + 0.2f, pv.gain[i] > 0 || pv.room[i] > 0 ? NsDraw::GREEN : dim, b);
        }
    }
    ry += 10 * 6.6f + 2;
    /* virtues and saints */
    if (c.nvirtue || c.nsaint) {
        std::string v;
        for (int i = 0; i < c.nvirtue; i++) { if (i) v += ", "; v += ns_gtext(c.virtue[i]); }
        if (!v.empty()) ry += P.para(fo, 4.2f, RX0 + 2.5f, ry, RX1 - RX0 - 5, soft, "Virtues: " + v) + 1;
        std::string s;
        for (int i = 0; i < c.nsaint; i++) { int k = c.saint[i]; if (k >= 0 && k < (int)gamedata.saints.size()) { if (!s.empty()) s += ", "; s += gamedata.saints[k].name; } }
        if (!s.empty() && ry < 192) P.para(fo, 4.2f, RX0 + 2.5f, ry, RX1 - RX0 - 5, soft, "Saints known: " + s);
    }
}
