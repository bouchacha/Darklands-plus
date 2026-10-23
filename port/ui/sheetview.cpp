/* sheetview.cpp - draws a member's sheet (mods/sheetview.c runs it; docs/NATIVE_SCREENS.md),
 * the whole page anew beside the party panel.
 *
 * Top: the member's name, sex and age, the leader, the tabs, Close.
 * The one place for the party's things, skills, saints and formulae (the F12 window has no pages
 * of its own for them).
 * Things: the paper doll on its column (put together from the game's own pictures, as the
 *   original does: PICS\ARMSBACK.PIC and the armour, weapon, shield and missile weapon over it),
 *   the load under it; right, the things, those in use first, the quality only where it can
 *   differ; Ready / Unequip / Drink and Drop on the row under the pointer, Undo for what was
 *   dropped; a thing dragged onto a member at the left is given to them (Shift: one of a stack);
 *   a weapon, armour or shield the pointer rests on shows all its numbers and who could use it.
 * Skills: the doll; right, the attributes and the skills on one page in the original's groups,
 *   each with its sign; Endurance, Strength and Divine Favor as current/maximum, the others
 *   their value (with the maximum, in red, when lowered); blue what a saint raised for a while;
 *   a gold dot where the member is the party's best. The strip: what it is for, how long a
 *   saint's change lasts, the whole party's values; with nothing under the pointer, what is at
 *   work on the member.
 * Saints: the whole party's - each saint someone knows (or all), who knows it, a choice of what
 *   they help with, "taught here" by the library the party is at; the saint under the pointer (or
 *   clicked) with the game's statue for it (its prayer screen's picture), who knows it or the
 *   libraries the party has been to that teach it, what it does (mods/saintinfo.h), where the
 *   text says otherwise, whether encounters call on it, and the legend.
 * Formulae: the whole party's - each formula someone knows (or all), who knows it, the batches
 *   its ingredients make, "sold here"; under the list the 23 ingredients the party has, those
 *   the formula shown calls for ringed; the formula under the pointer with its effect, mystic
 *   number and risk, its ingredients against what the party carries, and each alchemist's
 *   chance with one batch, or where an alchemist the party has been to sells it. */
#include "imgui.h"
#include "skillicons.h"
#include "../runtime/config.h"
#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../mods/sheetview.h"
#include "../mods/gamemodel.h"
#include "../mods/saintinfo.h"
#include "nsdraw.h"
#include "libraries.h"
#include "money.h"
#include "effects.h"
bool ui_item_tidy_less(int ida, int qa, int idb, int qb);    /* (inventory.cpp) */
#include "gamepics.h"

extern "C" u32 plat_ticks_ms(void);
extern "C" int partypanel_member_at(int gx, int gy);
extern int partypanel_focus;
void ui_item_icon(ImDrawList *dl, ImVec2 c, float s, int id);
const char *ui_attr_help(int a);
const char *ui_skill_help(int k);
std::string rules_weapons_of_skill(int k);
size_t saint_effect_at(const std::string &text);
std::string journal_where_sold(int id);                     /* (ui/journal.cpp) */
void ui_item_details(const ItemDef *d, int id, int q);      /* (ui/inventory.cpp) */

namespace {
enum { TAB_THINGS, TAB_SKILLS, TAB_SAINTS, TAB_FORMULAE };
/* what the saints help with: words of their effect */
struct Use { const char *name; const char *words[12]; };
const Use USES[] = {
    { "Any", { nullptr } },
    { "Healing", { "heal", "cure", "restor", "wound", "disease", "poison", "endurance", nullptr } },
    { "Strength", { "strength", nullptr } },
    { "Fighting", { "weapon", "attack", "combat", "battle", "fight", "defen", "damage", "edged", "impact", "flail", "polearm", nullptr } },
    { "Missiles", { "thrown", "bow", "missile", "arrow", nullptr } },
    { "Protection", { "protect", "fire", "safe", "shield", "armo", nullptr } },
    { "Travel", { "travel", "journey", "water", "ship", "sea", "riding", "horse", "woodwise", nullptr } },
    { "Mind", { "intellig", "perception", "latin", "read", "alchemy", "artifice", "knowledge", nullptr } },
    { "Charm", { "charisma", "speak", "reputation", "virtue", "streetwise", nullptr } },
    { "Stealth", { "stealth", "agility", "hid", nullptr } },
};
const int NUSES = (int)(sizeof USES / sizeof USES[0]);

struct SheetView {
    int tab = TAB_THINGS;
    int scroll[4] = {};
    int last_who = -1;
    /* the strip: 1 an item, 2 a skill, 3 a formula, 4 a saint, 5 an attribute */
    int what = 0, which = -1; unsigned seq = 0;
    int drop_row = -1; unsigned act_seq = 0;      /* a row asking "sure?" */
    int pin_saint = -1, pin_formula = -1;         /* clicked: stays while the pointer is elsewhere */
    bool saints_all = false, formulae_all = false; int use = 0;
    int tip_row = -1; unsigned tip_t = 0;        /* a thing's row under the pointer, since when (its details after a moment) */
    /* dragging a thing */
    int drag_row = -1; bool dragging = false;
} V;

std::string lower(std::string s) { for (char &c : s) c = (char)tolower((unsigned char)c); return s; }

const ImU32 BEST = IM_COL32(196, 146, 30, 255), RAISED = IM_COL32(26, 64, 132, 255);   /* the party's best; raised for a while */

/* what is at work on the party now (ui/effects.h), read once a frame */
std::vector<Eff> EFF;
std::string hours_left(long h)
{
    char b[32];
    if (h < 48) snprintf(b, sizeof b, "%ld hour%s", h, h == 1 ? "" : "s");
    else snprintf(b, sizeof b, "%ld days", (h + 12) / 24);
    return b;
}
/* a saint's (or its price's) change to member m's attribute (kind 0x48) or skill (0x49) k: the sum,
   and the soonest it ends */
int raised_by(int m, int kind, int k, long *left)
{
    int t = 0; long l = -1;
    for (const Eff &e : EFF) if (e.m == m && e.kind == kind && e.k == k) { t += e.amount; if (l < 0 || e.left < l) l = e.left; }
    if (left) *left = l;
    return t;
}

/* the doll's pictures (s031 1462:0b22) */
std::vector<std::string> doll_layers(const SheetModel &s)
{
    std::vector<std::string> v = { "PICS\\ARMSBACK.PIC" };
    switch (s.limb) {
    case 0x4c: case 0x4d: v.push_back("PICS\\PAD-LIMS.PIC"); break; case 0x4e: v.push_back("PICS\\LEAT-LIM.PIC"); break;
    case 0x4f: v.push_back("PICS\\STUDLIM.PIC"); break; case 0x50: v.push_back("PICS\\CUIRBLIM.PIC"); break;
    case 0x51: v.push_back("PICS\\SCALELIM.PIC"); break; case 0x52: v.push_back("PICS\\CHAINLIM.PIC"); break;
    case 0x53: v.push_back("PICS\\BRIGLIM.PIC"); break; case 0x54: v.push_back("PICS\\PLATELIM.PIC"); break;
    }
    switch (s.vital) {
    case 0x43: case 0x44: v.push_back("PICS\\PAD-VIT.PIC"); break; case 0x45: v.push_back("PICS\\LEAT-VIT.PIC"); break;
    case 0x46: v.push_back("PICS\\STUDVIT.PIC"); break; case 0x47: v.push_back("PICS\\CUIRBVIT.PIC"); break;
    case 0x48: v.push_back("PICS\\SCALEVIT.PIC"); break; case 0x49: v.push_back("PICS\\CHAINVIT.PIC"); break;
    case 0x4a: v.push_back("PICS\\BRIGVIT.PIC"); break; case 0x4b: v.push_back("PICS\\PLATEVIT.PIC"); break;
    }
    static const signed char WPN[34] = { 2, 3, 4, 5, 6, 7, 8, 0, 9, 1, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 24, 0,
                                         26, 27, 1, 28, 29, 30, 31, 32, 33, 34 };
    char b[32];
    if (s.weapon >= 0 && s.weapon < 34) { snprintf(b, sizeof b, "PICS\\WEAPON%d.PIC", WPN[s.weapon]); v.push_back(b); }
    if (s.shield >= 0) v.push_back(s.shield == 0x5f ? "PICS\\SMALLSH.PIC" : "PICS\\MEDIUMSH.PIC");
    if (s.missile >= 23 && s.missile <= 33) {
        static const signed char MSL[11] = { 23, 26, 27, 1, 28, 29, 30, 31, 32, 33, 34 };
        snprintf(b, sizeof b, "PICS\\WEAPON%d.PIC", MSL[s.missile - 23]); v.push_back(b);
    }
    return v;
}

bool shows_quality(const ItemDef *d, int q) { return d && ns_shows_quality((int)(d - &gamedata.items[0]), q); }

const char *LOAD[4] = { "Light load", "Normal load", "Heavy load", "Overloaded" };
/* what each band does, from the game's code (equip_weight s031 18e7:140a: the equipped things'
   weights; s014 18bf: attack_rate adds 30 to a melee blow's time between half and all of
   Endurance + Strength, 100 beyond half again - nothing between; the dodge keeps the whole
   Agility up to 5/8 of it, two thirds up to 1 1/4, a third up to 1 7/8, then none) */
/* the row of a thing under the pointer (the strip shows it), -1 none: 1..5 give it (mods/sheetview.c) */
extern "C" int sheetview_pointed_item(void);
extern "C" const char *ui_load_effect(int band)
{
    switch (band) {
    case 0: return "Light load (up to half of Endurance + Strength): no hindrance at all. Only what is equipped counts: the weapons, armour and shield in use.";
    case 1: return "Normal load (up to Endurance + Strength): blows hand to hand come about a fifth slower, and past 5/8 of it the Agility used to dodge falls to two thirds.";
    case 2: return "Heavy load (up to half as much again): the Agility used to dodge falls to two thirds, and to a third past 1 1/4 times Endurance + Strength. Blows are no slower than with a normal load (the game forgets to slow them in this band).";
    default: return "Overloaded (more than 1 1/2 times Endurance + Strength): blows hand to hand come about two thirds slower, and the Agility used to dodge falls to a third, then to nothing past about twice Endurance + Strength.";
    }
}

/* a saint's description: legend, then what it does */
void saint_parts(const std::string &t, std::string &lore, std::string &eff)
{
    size_t a = saint_effect_at(t);
    lore = t.substr(0, a); size_t b = a; while (b < t.size() && t[b] == ' ') b++;
    eff = t.substr(b);
    while (!lore.empty() && lore.back() == ' ') lore.pop_back();
}
bool saint_fits(int s, int use)
{
    if (use <= 0) return true;
    std::string lore, eff; saint_parts(gamedata.saints[s].text, lore, eff);
    std::string e = lower((eff.empty() ? gamedata.saints[s].text : eff) + " " + saint_info_does(s));   /* (what it promises, what it does) */
    for (const char *const *w = USES[use].words; *w; w++) if (e.find(*w) != std::string::npos) return true;
    return false;
}
const GamePic *statue(int cls)
{
    int k = cls >= 0 && cls <= 9 ? cls : cls >= 10 && cls <= 13 ? 0 : 1;
    char b[32]; snprintf(b, sizeof b, "PICS\\SNTBACK%d.PIC", k);
    return gamepic_part(b, 70, 0, 70, 196);
}
}

extern "C" int sheetview_showing(void) { return ns_screen == NS_SHEET && sheet_model.open && !ns_hidden(); }

static void header(NsDraw &P, const SheetModel &s)
{
    ImFont *f = P.f, *fo = P.fo;
    P.text(f, NsDraw::FS_HEAD, 63.5f, 2.2f, NsDraw::INK, ns_gtext(s.fullname), -1, 290);
    if (P.button(296, 2.5f, 318, 10.2f, "Close", IM_COL32(112, 40, 22, 235), NsDraw::FS_SMALL, PB_Y)) ns_push(SV_CLOSE, 0, 0, 0);
    char sub[96]; snprintf(sub, sizeof sub, "%s, age %d", s.female ? "Woman" : "Man", s.age);
    for (const Eff &e : EFF) if (e.m == s.who && e.kind == 0x43) { snprintf(sub + strlen(sub), sizeof sub - strlen(sub), "; leaves the party in %s", hours_left(e.left).c_str()); break; }
    P.text(fo, NsDraw::FS_SMALL, 63.5f, 12.4f, NsDraw::INK_SOFT, sub);
    float lx = 63.5f + P.width(fo, NsDraw::FS_SMALL, sub) + 3;
    if (s.who == s.leader) {
        P.rect(lx, 11.6f, lx + 17, 18.8f, IM_COL32(214, 172, 60, 255), 1.2f);
        P.text(fo, NsDraw::FS_SMALL, lx + 2, 12.4f, NsDraw::INK, "Leader");
    } else if (P.button(lx, 11.6f, lx + 26, 18.8f, "Make leader", IM_COL32(120, 84, 40, 235), 4.4f)) ns_push(SV_LEADER, s.who, 0, 0);
    if (P.over(lx, 11.6f, lx + (s.who == s.leader ? 17 : 26), 18.8f))
        ns_tip(config_get_int("assist.best_leader", 1) && !config_get_int("original", 0)
            ? "On cards, each test is made by the member best at it, and shops set prices by the best bargainer, whoever leads. "
              "The leader is still the one meant by \"the leader\" on a card (a mine's traps, a church's favour, an item handed over), "
              "and in battles and dungeons (when not paused) deals with doors, chests, locks and traps and leads the group's movement."
            : "The leader speaks and acts for the party on the cards and bargains in the shops.");
    static const char *TABS[4] = { "Items", "Skills", "Saints", "Formulae" };
    if (sheet_model.tab_step) {                             /* (Tab, Shift+Tab, Right, Left: mods/sheetview.c) */
        V.tab = ((V.tab + sheet_model.tab_step) % 4 + 4) % 4; V.drop_row = -1; V.what = 0;
        sheet_model.tab_step = 0;
    }
    float tw = 25.5f, x = 318 - 4 * tw;
    for (int t = 0; t < 4; t++) {
        bool sel = t == V.tab, h = P.over(x, 11, x + tw - 0.8f, 19.5f);
        P.tab(x, 11, x + tw - 0.8f, 19.5f, sel, h);
        P.text(f, 5.2f, x + (tw - 0.8f - P.width(f, 5.2f, TABS[t])) * 0.5f, 12.3f, sel ? NsDraw::LIGHT : NsDraw::INK, TABS[t]);
        if (P.hit(x, 11, x + tw - 0.8f, 19.5f)) { V.tab = t; V.drop_row = -1; V.what = 0; }
        x += tw;
    }
    P.rect(62, 20.3f, 318, 20.7f, IM_COL32(120, 80, 36, 110));
}

/* what the load does, for this member now (the game's rules: member_eff_attr2 and the band, s029
   0e76:013a, for the tests on cards; to_hit_roll and attack_rate, s014 18bf, in melee) */
static std::string load_effects(const SheetModel &s)
{
    int L = s.load, cap = s.attr[0] + s.attr[1], capm = s.attr_max[0] + s.attr_max[1], agl = s.attr[2], m = 5 * capm / 4;
    auto part = [](int l, int c, int a, bool one) -> std::pair<int, const char *> {
        if (l <= c / 2) return { a, "in full" };
        if (l <= c) return { 2 * a / 3, "two thirds" };
        if (l <= 3 * c / 2) return { a / 3, "a third" };
        return { one ? 1 : 0, one ? "as 1" : "not at all" };
    };
    auto card = part(L, cap, agl, true), def = part(L, m, agl, false);
    const char *swing = (L > capm / 2 && L <= capm) ? "a little slower (the attack rate falls)" : L > 3 * capm / 2 ? "much slower (the attack rate falls a lot)" : "at the usual pace";
    char b[900];
    snprintf(b, sizeof b,
             "Load %d: the weight of what %s has equipped (weapons, armour, shield). Spare items in the pack weigh nothing.\n"
             "Compared with Endurance + Strength (%d now): light up to %d, normal up to %d, heavy up to %d, overloaded above that.\n\n"
             "Now:\n"
             "- Tests on cards that ask for Agility (climbing a wall, running, a trap): Agility %d counts %s (%d).\n"
             "- Fending off blows in melee: Agility counts %s (%d).\n"
             "- Swinging in melee: %s.\n\n"
             "Travel, walking in a battle, missiles and Endurance are not affected by the load.",
             L, ns_gtext(s.names[s.who]).c_str(), cap, cap / 2, cap, 3 * cap / 2, agl, card.second, card.first, def.second, def.first, swing);
    return b;
}

static void doll(NsDraw &P, const SheetModel &s)
{
    if (false) {                                            /* (the stone niche drawn anew, ui/niche.cpp: set aside, the player preferring the game's own) */
        void ui_draw_doll_niche(ImDrawList *dl, ImVec2 o, float kx, float k);
        ui_draw_doll_niche(P.dl, NsDraw::w(62, 21.5f), P.kx, P.k);
        const GamePic *d = gamepic_layers_off_back(doll_layers(s), "PICS\\ARMBACK.PIC");
        if (d) P.dl->AddImage((ImTextureID)(intptr_t)d->tex, NsDraw::w(62, 21.5f), NsDraw::w(62 + d->w, 21.5f + d->h));
    } else {
    const GamePic *g = gamepic_layers(doll_layers(s), "PICS\\ARMBACK.PIC");
    if (g) P.dl->AddImage((ImTextureID)(intptr_t)g->tex, NsDraw::w(62, 21.5f), NsDraw::w(62 + g->w, 21.5f + g->h));
    else P.rect(62, 21.5f, 198, 191.5f, IM_COL32(200, 196, 190, 255));
    }
    /* the load */
    int band = s.load_band < 0 ? 0 : s.load_band > 3 ? 3 : s.load_band;
    ImU32 c = band >= 3 ? NsDraw::RED : band == 2 ? IM_COL32(150, 80, 20, 255) : NsDraw::INK;
    P.text(P.fo, NsDraw::FS_SMALL, 63.5f, 192.3f, c, LOAD[band]);
    if (P.over(62, 191, 198, 197.5f)) ns_tip(ui_load_effect(band), 26);
    char v[24]; snprintf(v, sizeof v, "%d of %d", s.load, s.capacity);
    P.text(P.fo, NsDraw::FS_SMALL, 0, 192.3f, NsDraw::INK_SOFT, v, 197);
    float f = s.capacity > 0 ? fminf(1.5f, (float)s.load / s.capacity) / 1.5f : 0;
    P.rect(100, 194.6f, 160, 196.4f, IM_COL32(214, 196, 160, 255), 0.6f);
    P.rect(100, 194.6f, 100 + 60 * f, 196.4f, c, 0.6f);
    P.rect(100 + 60 / 3.0f, 194.2f, 100 + 60 / 3.0f + 0.4f, 196.8f, IM_COL32(120, 80, 36, 160));
    P.rect(100 + 120 / 3.0f, 194.2f, 100 + 120 / 3.0f + 0.4f, 196.8f, IM_COL32(120, 80, 36, 160));
    if (P.over(62, 191.5f, 198, 198) && config_get_int("assist.shop_info", 1)) {   /* what the load does */
        ns_tip(load_effects(s).c_str(), 25);
    }
}

/* where the shown member stands in the party at attribute or skill idx: 1 the best (others lower),
   0 not, -1 nobody to compare with */

/* the strip's line on a saint's change to the attribute or skill under the pointer */
static float raised_line(NsDraw &P, const SheetModel &s, bool attr, int idx, float x, float y, float w)
{
    long left; int t = raised_by(s.who, attr ? 0x48 : 0x49, idx, &left);
    if (!t) return 0;
    char b[96]; snprintf(b, sizeof b, "%s by %d for %s more (a saint's %s).", t > 0 ? "Raised" : "Lowered", t > 0 ? t : -t, hours_left(left).c_str(), t > 0 ? "help" : "price");
    return P.para(P.fo, NsDraw::FS_SMALL, x, y, w, t > 0 ? RAISED : NsDraw::RED, b) + 0.6f;
}
static int party_best(const SheetModel &s, bool attr, int idx)
{
    int mine = attr ? s.pattr[s.who][idx] : s.pskill[s.who][idx], n = 0, lower = 0;
    for (int m = 0; m < 5; m++) {
        if (!s.present[m] || m == s.who) continue;
        int v = attr ? s.pattr[m][idx] : s.pskill[m][idx];
        n++;
        if (v > mine) return 0;
        if (v < mine) lower++;
    }
    return n ? (lower ? 1 : 0) : -1;
}

/* the whole party at attribute or skill idx, in a line under the strip's text (what F12 > Party
   once showed side by side): each name and value, the best in gold, the shown member underlined */
static float party_line(NsDraw &P, const SheetModel &s, bool attr, int idx, float x0, float y, float x1)
{
    int hi = -1000, lo = 1000, n = 0;
    for (int m = 0; m < 5; m++) if (s.present[m]) { int v = attr ? s.pattr[m][idx] : s.pskill[m][idx]; hi = std::max(hi, v); lo = std::min(lo, v); n++; }
    if (n < 2) return 0;
    const float gs = NsDraw::FS_SMALL;
    float x = x0, lh = gs * 1.25f;
    P.text(P.fo, gs, x, y, NsDraw::INK_SOFT, "The party:"); x += P.width(P.fo, gs, "The party:") + 1.5f;
    for (int m = 0; m < 5; m++) {
        if (!s.present[m]) continue;
        int v = attr ? s.pattr[m][idx] : s.pskill[m][idx], mx = attr ? s.pattr_max[m][idx] : v;
        std::string seg = ns_gtext(s.names[m]) + " " + (v < mx ? std::to_string(v) + "/" + std::to_string(mx) : std::to_string(v));
        float w = P.width(P.fo, gs, seg);
        if (x + w > x1) { x = x0 + 2; y += lh; }
        bool best = hi > lo && v == hi;
        if (best) P.dl->AddCircleFilled(NsDraw::w(x + 0.9f, y + gs * 0.62f), 0.8f * P.k, BEST);
        float tx = best ? x + 2.4f : x;
        P.text(P.fo, gs, tx, y, best ? IM_COL32(130, 92, 10, 255) : NsDraw::INK, seg);
        if (m == s.who) P.rect(tx, y + gs * 1.12f, tx + w, y + gs * 1.12f + 0.35f, NsDraw::INK_SOFT);
        x = tx + w + 3;
    }
    return y + lh;
}

/* the strip at the bottom right: what the pointer is on */
static void strip(NsDraw &P, const SheetModel &s, float SX0, float SX1, float SY0, float SY1)
{
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM;
    float SW = SX1 - SX0 - 5, y = SY0;
    P.rect(SX0, SY0 - 1.5f, SX1, SY1, IM_COL32(230, 214, 178, 255), 1.5f);
    if (V.what == 1 && V.which >= 0 && V.which < s.nitems) {
        const SvItem &it = s.item[V.which];
        const ItemDef *d = ns_itemdef(it.id);
        y += P.para(f, NsDraw::FS_NAME, SX0 + 2.5f, y, SW, ink, ns_item_name(it.id)) + 0.5f;
        std::string l1 = std::string(d ? kind_name_one(d->kind) : "");
        if (shows_quality(d, it.qual)) l1 += ns_quality_words(it.id, it.qual);
        bool wearable = d && (d->kind == K_WEAPON || d->kind == K_MISSILE || d->kind == K_ARMOR || d->kind == K_SHIELD);   /* (only equipped things count toward the load: equip_weight) */
        if (wearable) l1 += ", weight " + std::to_string(it.weight) + (it.equipped ? "" : " when equipped");
        if (it.count > 1) l1 += std::string(wearable ? " each" : "") + ", " + std::to_string(it.count) + " of them";
        if (it.equipped == 2) l1 += "; this member's mount";
        else if (it.equipped) l1 += "; equipped";
        else if (it.holder < 0) l1 += it.count > 1 ? " in the party's pack" : "; in the party's pack";
        if (it.kept) l1 += "; kept (not sold or dropped)";
        y += P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y, SW, soft, l1) + 0.5f;
        std::string l2 = ns_item_line(it.id, it.qual);
        if (!l2.empty()) y += P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y, SW, soft, l2) + 0.5f;
        std::string use = ns_item_use(it.id);
        if (!use.empty()) y += P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y, SW, soft, use) + 0.5f;
        { ImU32 ec; std::string enc = ns_item_encounters(it.id, &ec); if (!enc.empty()) y += P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y, SW, ec, enc) + 0.5f; }
        if (it.holder >= 0 && !it.equipped && s.npar > 1)          /* (how to hand it over: the pack's spares are anyone's already) */
            y += P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y, SW, NsDraw::INK_DIM, it.count > 1 ? "To give the lot, drag it onto a member at the left, or press their number (1 to 5); Shift: just one." : "To give it, drag it onto a member at the left, or press their number (1 to 5).") + 0.5f;
        if (d && d->type >= 0 && d->type <= 33 && (d->kind == K_WEAPON || d->kind == K_MISSILE)) {
            int sk = rb(G_FAR20A5, (u16)(0x75ac + d->type)), minsk = rb(G_FAR20A5, (u16)(0x7726 + d->type)), minst = rb(G_FAR20A5, (u16)(0x7765 + d->type));
            int have = sk < 7 ? s.skill[sk] : 0;
            std::string fit = (have < minsk ? "needs " + std::to_string(minsk - have) + " more skill" : std::string("")) +
                              (s.attr[1] < minst ? std::string(have < minsk ? ", " : "") + "too weak by " + std::to_string(minst - s.attr[1]) : "");
            P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y, SW, fit.empty() ? NsDraw::GREEN : NsDraw::RED, fit.empty() ? ns_gtext(s.names[s.who]) + " can use it well" : ns_gtext(s.names[s.who]) + ": " + fit);
        } else if (d && d->type >= 95 && d->type <= 97) {     /* a shield: the defence it gives this member (mods/gamemodel.h) */
            int sb = gm_shield_defence(d->type, it.qual, 0), sh = gm_shield_defence(d->type, it.qual, 1), ag = gm_member_fight_agility(s.who), a = s.attr[2];
            char b[240];
            snprintf(b, sizeof b, "%s's defence with it: against a blow, %d + their melee rating; against a shot or throw, %d + twice the Agility %d = %d%s", ns_gtext(s.names[s.who]).c_str(), sb, sh, ag, sh + 2 * ag,
                     ag < a ? " (the load lowers Agility)" : "");
            P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y, SW, ink, b);
        } else if (l2.empty() && it.drinkable && it.id >= 95 && 3 * (it.id - 95) < (int)gamedata.formulas.size())
            P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y, SW, soft, game_text(gamedata.formulas[3 * (it.id - 95)].text.c_str(), gamedata.formulas[3 * (it.id - 95)].text.size()));
    } else if (V.what == 2 && V.which >= 0 && V.which < 19) {
        std::string help = ui_skill_help(V.which);
        if (V.which < 7) { std::string w = rules_weapons_of_skill(V.which); help = (V.which >= 4 ? "Aim with " : "Attack and defence hand to hand with ") + (w.empty() ? std::string("these weapons") : w) + "."; }
        y += P.para(f, NsDraw::FS_NAME, SX0 + 2.5f, y, SW, ink, gm_skill_name(V.which)) + 0.5f;
        y += P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y, SW, soft, help) + 0.8f;
        y += raised_line(P, s, false, V.which, SX0 + 2.5f, y, SW);
        party_line(P, s, false, V.which, SX0 + 2.5f, y, SX1 - 2);
    } else if (V.what == 5 && V.which >= 0 && V.which < 7) {
        y += P.para(f, NsDraw::FS_NAME, SX0 + 2.5f, y, SW, ink, gm_attr_name(V.which)) + 0.5f;
        y += P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y, SW, soft, ui_attr_help(V.which)) + 0.8f;
        y += raised_line(P, s, true, V.which, SX0 + 2.5f, y, SW);
        party_line(P, s, true, V.which, SX0 + 2.5f, y, SX1 - 2);
    } else if (V.tab == TAB_THINGS && s.shared)
        P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y + 1, SW, dim, "Point at an item to see it here; F1 shows what you can do.");
    else if (V.tab == TAB_THINGS)
        P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y + 1, SW, dim, "Point at an item to see it here. Drag an item onto a member at the left to give it to them (Shift: just one of a stack). Click another member at the left to see their sheet; Esc or Close goes back.");
    else {
        y += P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y + 1, SW, dim, s.npar > 1 ? "Point at an attribute or a skill to see what it does and the whole party's values. Gold dot: the party's best at it; blue: raised by a saint for a while."
                                                                                : "Point at an attribute or a skill to see what it does. Blue: raised by a saint for a while.") + 1.8f;
        std::vector<std::string> at;                     /* the blessings and potions at work on this member, as many as there is room for */
        for (const Eff &e : EFF) if (e.m == s.who && e.kind != 0x43) at.push_back(e.what + " (" + hours_left(e.left) + ")");
        if (!at.empty()) {
            std::string t = "In effect now: ";
            size_t k = 0;
            auto tail = [&](size_t left) { return left ? "; and " + std::to_string(left) + " more (see In force in the margin)." : std::string("."); };
            for (; k < at.size(); k++) {
                std::string next = t + (k ? "; " : "") + at[k];
                if (k && P.para_height(fo, NsDraw::FS_SMALL, SW, next + tail(at.size() - k - 1)) > SY1 - y - 1) break;
                t = next;
            }
            t += tail(at.size() - k);
            P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y, SW, RAISED, t);
        }
    }
    if (s.note[0] && plat_ticks_ms() - s.note_t < 6000) {
        P.rect(SX0 + 1, SY1 - 8.6f, SX1 - 1, SY1 - 0.6f, IM_COL32(230, 214, 178, 255));
        P.text(fo, NsDraw::FS_SMALL, SX0 + 2.5f, SY1 - 7.8f, IM_COL32(112, 40, 22, 255), ns_gtext(s.note), -1, SX1 - 1);
    }
}

/* the details of a thing in use in a fight (ui_item_details: the numbers, who in the party could
   use it, what armour stops), and about what a shop pays for it */
static void gear_tip(const SvItem &it)
{
    const ItemDef *d = ns_itemdef(it.id);
    if (!d || !((d->type >= 0 && d->type <= 33 && (d->kind == K_WEAPON || d->kind == K_MISSILE || d->kind == K_RELIC)) ||
                (d->type >= 67 && d->type <= 84) || (d->type >= 95 && d->type <= 97))) return;
    ns_tip_begin(30);
    ImGui::TextUnformatted(ns_item_name(it.id).c_str());
    if (d->value) ImGui::TextDisabled("Worth about %s", money_with_total(d->value).c_str());
    ui_item_details(d, it.id, it.qual);
    ns_tip_end();
}

static void things(NsDraw &P, const SheetModel &s, int &new_what, int &new_which)
{
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM;
    const float RX0 = 200, RX1 = 318, LY0 = 21.5f, ROWH = 8.0f; const int NROWS = 16;
    const float LY1 = LY0 + NROWS * ROWH;
    /* the rows: a heading, then those in use; a heading, then the rest */
    std::vector<int> rows;                                 /* item index; -1 / -3 / -2 the headings */
    int nin = 0, npot = 0;
    for (int i = 0; i < s.nitems; i++) { if (s.item[i].equipped) nin++; else if (s.item[i].potion) npot++; }
    /* each section in the tidy order of the party's inventory page: by kind, then name, the better first */
    auto tidy = [&](size_t from) { std::stable_sort(rows.begin() + from, rows.end(), [&](int a, int b) { return ui_item_tidy_less(s.item[a].id, s.item[a].qual, s.item[b].id, s.item[b].qual); }); };
    if (nin) { rows.push_back(-1); size_t f = rows.size(); for (int i = 0; i < s.nitems; i++) if (s.item[i].equipped) rows.push_back(i); tidy(f); }
    if (npot) { rows.push_back(-3); size_t f = rows.size(); for (int i = 0; i < s.nitems; i++) if (!s.item[i].equipped && s.item[i].potion) rows.push_back(i); tidy(f); }
    if (s.nitems > nin + npot) { rows.push_back(-2); size_t f = rows.size(); for (int i = 0; i < s.nitems; i++) if (!s.item[i].equipped && !s.item[i].potion) rows.push_back(i); tidy(f); }
    int n = (int)rows.size();
    int &sc = V.scroll[TAB_THINGS];
    if (ns_in.wheel != 0) { sc -= (int)(ns_in.wheel * 2); ns_in.wheel = 0; }
    int maxs = n - NROWS; if (maxs < 0) maxs = 0;
    if (sc > maxs) sc = maxs;
    if (sc < 0) sc = 0;
    P.rect(RX0, LY0 - 0.5f, RX1, LY1 + 0.5f, IM_COL32(250, 242, 222, 255), 1.5f);
    if (!s.nitems) P.text(fo, NsDraw::FS_BODY, RX0 + 3, LY0 + 2, dim, "Carries nothing.");
    /* a press on a row may become a drag */
    if (ns_in.pressed) {
        ns_in.pressed = 0;
        V.drag_row = -1;
        if (ns_in.px >= RX0 && ns_in.px < RX1 - 40 && ns_in.py >= LY0 && ns_in.py < LY1) {
            int r = (int)((ns_in.py - LY0) / ROWH) + sc;
            if (r >= 0 && r < n && rows[r] >= 0 && s.item[rows[r]].holder >= 0) V.drag_row = rows[r];   /* (the shared pack's rows are nobody's to give) */
        }
    }
    if (V.drag_row >= 0 && ns_in.down && !V.dragging && (abs(ns_in.x - ns_in.px) + abs(ns_in.y - ns_in.py)) > 4) { V.dragging = true; sheetview_dragging = 1; }
    if (!ns_in.down && !P.click && V.dragging) { V.dragging = false; sheetview_dragging = 0; V.drag_row = -1; }
    if (V.dragging && P.click) {                         /* let go: on a member at the left, a give */
        int to = partypanel_member_at(P.cx, P.cy);
        if (to >= 0 && to != s.who && V.drag_row < s.nitems) ns_push(SV_GIVE, s.item[V.drag_row].idx, to, P.shift ? 1 : 255);   /* (the member's record, not the row) */
        P.click = false; V.dragging = false; sheetview_dragging = 0; V.drag_row = -1;
    }
    int tip_i = -1;
    const bool cmp_on = config_get_int("display.shop_compare", 1) != 0;
    static int hover_carried = -1;                         /* (the carried row pointed at last frame: what it would replace is lit) */
    int cmp_slot = cmp_on && hover_carried >= 0 && hover_carried < s.nitems && !s.item[hover_carried].equipped ? ns_slot_of(s.item[hover_carried].id) : 0;
    hover_carried = -1;
    for (int r = 0; r < NROWS && sc + r < n; r++) {
        int i = rows[sc + r];
        float y = LY0 + r * ROWH;
        if (i < 0) {                                       /* a heading */
            const char *hd = i == -1 ? "Equipped" : i == -3 ? "Potions (the marks on a flask: its grade, 1 to 3)" : s.shared ? "The party's pack" : "Carried";
            P.text(fo, NsDraw::FS_SMALL, RX0 + 3, y + 2.2f, soft, hd);
            P.rect(RX0 + 3 + P.width(fo, NsDraw::FS_SMALL, hd) + 2, y + 4.8f, RX1 - 3, y + 5.1f, IM_COL32(120, 80, 36, 90));
            continue;
        }
        const SvItem &it = s.item[i];
        bool h = !V.dragging && P.over(RX0, y, RX1, y + ROWH), busy = i == V.drop_row;
        if (h) { new_what = 1; new_which = i; hover_carried = i; }
        if (h || busy || (V.dragging && i == V.drag_row)) P.rect(RX0 + 0.5f, y, RX1 - 0.5f, y + ROWH, IM_COL32(226, 204, 160, 255));
        P.icon(RX0 + 5.5f, y + ROWH * 0.5f, 6.3f, it.id, it.qual);
        std::string nm = ns_item_name(it.id);
        if (it.count > 1) nm += " (" + std::to_string(it.count) + ")";
        const ItemDef *d = ns_itemdef(it.id);
        float name_end = (h || busy) ? RX1 - 59 : RX1 - 2;
        if (h && P.mx < name_end) tip_i = i;
        if (it.kept) {                                     /* kept from selling and dropping: a small seal at the right */
            float kx = (h || busy) ? RX1 - 61 : RX1 - 4;
            P.dl->AddCircleFilled(NsDraw::w(kx, y + ROWH * 0.5f), 1.5f * P.k, IM_COL32(150, 40, 24, 230));
            name_end -= 4;
        }
        /* (display.shop_compare) a weapon, armour or shield carried: better or worse than what this member has there now,
           as in a shop; the one pointed at lights what it would replace */
        int vs = !it.equipped && cmp_on ? ns_against_worn(it.id, it.qual, s.who) : 0;
        if (it.equipped == 1 && cmp_slot && ns_slot_of(it.id) == cmp_slot) {
            float pl = 0.5f + 0.5f * sinf(plat_ticks_ms() * 0.008f);
            P.rect(RX0 + 0.5f, y, RX1 - 0.5f, y + ROWH, IM_COL32(214, 170, 60, (int)(40 + 50 * pl)));
        }
        float nw = P.width(f, NsDraw::FS_NAME, nm), qend = RX0 + 11 + nw;
        P.text(f, NsDraw::FS_NAME, RX0 + 11, y + 1.0f, ink, nm, -1, name_end - (vs ? 6 : 0));
        if (!ns_quality_tag(it.id, it.qual).empty()) {
            char qs[16]; snprintf(qs, sizeof qs, "%s", ns_quality_tag(it.id, it.qual).c_str());
            float qx = RX0 + 11 + nw + 2;
            if (qx + P.width(fo, NsDraw::FS_SMALL, qs) < name_end - (vs ? 6 : 0)) { P.text(fo, NsDraw::FS_SMALL, qx, y + 1.9f, soft, qs); qend = qx + P.width(fo, NsDraw::FS_SMALL, qs); }
        }
        if (vs) ns_compare_arrow(P.dl, fminf(qend + 4, name_end - 3), y + ROWH * 0.5f, vs, P.k);
        if (!(h || busy)) continue;
        /* the actions */
        bool gear = d && (d->kind == K_WEAPON || d->kind == K_MISSILE || d->kind == K_ARMOR || d->kind == K_SHIELD);
        bool pack = it.holder < 0;                         /* a row of the shared pack: the actions by kind and quality */
        float x = RX1 - 58;
        if (it.drinkable && !it.equipped) { if (P.button(x, y + 1, x + 18, y + ROWH - 1, "Drink", IM_COL32(52, 70, 120, 235), 4.4f)) { if (pack) ns_push(SV_PDRINK, it.id, it.qual, 0); else ns_push(SV_DRINK, it.idx, 0, 0); } }
        else if (it.equipped == 1) { if (P.button(x, y + 1, x + 18, y + ROWH - 1, "Unequip", IM_COL32(120, 84, 40, 235), 4.4f)) ns_push(SV_PUTAWAY, it.idx, 0, 0); }
        else if (it.equipped == 2) P.text(fo, NsDraw::FS_SMALL, x + 2, y + 2.4f, soft, "rides it");   /* (the shared pack: their own mount) */
        else if (gear) { if (P.button(x, y + 1, x + 18, y + ROWH - 1, "Ready", IM_COL32(40, 92, 34, 235), 4.4f)) { if (pack) ns_push(SV_PREADY, it.id, it.qual, 0); else ns_push(SV_READY, it.idx, 0, 0); } }
        x += 19;
        if (P.button(x, y + 1, x + 18, y + ROWH - 1, it.kept ? "Kept" : "Keep", it.kept ? IM_COL32(150, 40, 24, 245) : IM_COL32(120, 84, 40, 235), 4.4f)) ns_push(SV_KEEP, it.id, it.qual, !it.kept);
        if (P.over(x, y + 1, x + 18, y + ROWH - 1)) ns_tip(it.kept ? "Kept from selling and dropping (Sell spare gear skips it). Click to stop keeping it."
                                                                               : "Keep it: Darklands+ will not sell it (one by one or with Sell spare gear) or drop it until you click again.");
        x += 19;
        if (it.kept) P.button(x, y + 1, x + 18, y + ROWH - 1, "Drop", IM_COL32(170, 150, 130, 200), 4.4f);   /* (greyed: kept) */
        else if (busy) {
            if (P.button(x, y + 1, x + 18, y + ROWH - 1, P.shift || it.count == 1 ? "Sure?" : "Drop 1?", IM_COL32(150, 30, 18, 255), 4.4f)) {
                if (pack) ns_push(SV_PDROP, it.id, it.qual, P.shift ? 255 : 1); else ns_push(SV_DROP, it.idx, P.shift ? 255 : 1, 0);
                V.drop_row = -1;
            }
        } else if (P.button(x, y + 1, x + 18, y + ROWH - 1, "Drop", IM_COL32(120, 44, 24, 235), 4.4f)) { V.drop_row = i; V.act_seq = s.seq; }
    }
    if (n > NROWS) {
        float th = (LY1 - LY0) * NROWS / n, ty = LY0 + (LY1 - LY0 - th) * sc / (float)(n - NROWS);
        P.rect(RX1 - 1.4f, ty, RX1 - 0.3f, ty + th, IM_COL32(120, 80, 36, 140), 0.5f);
    }
    /* a weapon, armour or shield the pointer rests on: all its numbers, and the whole party's fit */
    if (tip_i != V.tip_row) { V.tip_row = tip_i; V.tip_t = plat_ticks_ms(); }
    else if (tip_i >= 0 && !V.dragging && plat_ticks_ms() - V.tip_t > 650) gear_tip(s.item[tip_i]);
    if (s.ndrops > 0) {                                  /* take back what was left behind here */
        const SvDrop &d = s.drops[s.ndrops - 1];
        std::string l = "Undo: " + ns_gtext(d.name) + (d.count > 1 ? " (" + std::to_string(d.count) + ")" : "");
        float bw = P.width(P.fo, 4.4f, l) + 4;
        if (P.button(RX1 - bw, LY1 + 1.2f, RX1, LY1 + 8.2f, l, IM_COL32(120, 84, 40, 235), 4.4f)) ns_push(SV_UNDO, 0, 0, 0);
    }
    /* the thing being dragged, at the pointer */
    if (V.dragging && V.drag_row >= 0 && V.drag_row < s.nitems) {
        const SvItem &it = s.item[V.drag_row];
        int to = partypanel_member_at(ns_in.x, ns_in.y);
        bool ok = to >= 0 && to != s.who;
        if (ok) P.dl->AddRect(NsDraw::w(1.2f, 40.0f * to + 1.2f), NsDraw::w(58.8f, 40.0f * to + 38.8f), IM_COL32(120, 220, 110, 255), 2 * P.k, 0, fmaxf(2.0f, 0.9f * P.k));
        float gx = ns_in.x + 5, gy = ns_in.y + 9;                  /* (below the pointer, not under it) */
        std::string l = ok ? "Give to " + ns_gtext(s.names[to]) + (P.shift || it.count == 1 ? "" : " (all " + std::to_string(it.count) + ")")
                           : ns_item_name(it.id) + (to == s.who ? "" : ": drop it on a member at the left");
        float lw = P.width(P.fo, NsDraw::FS_SMALL, l);
        if (gx + 12 + lw > 318) gx = 318 - 12 - lw;
        P.rect(gx, gy - 4, gx + 12 + lw + 2, gy + 4.5f, ok ? IM_COL32(222, 244, 214, 240) : IM_COL32(250, 242, 222, 235), 1.5f);
        P.icon(gx + 5, gy + 0.3f, 7, it.id, it.qual);
        P.text(P.fo, NsDraw::FS_SMALL, gx + 11, gy - 2.6f, NsDraw::INK, l);
    }
}

static void skills(NsDraw &P, const SheetModel &s, int &new_what, int &new_which)
{
    ImFont *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT;
    const float ROWH = 7.9f;
    P.rect(200, 21, 318, 150, IM_COL32(250, 242, 222, 255), 1.5f);
    struct Group { const char *name; int first, n; bool attr; };
    const Group COL1[2] = { { "Attributes", 0, 7, true }, { "Weapons", 0, 7, false } };
    const Group COL2[2] = { { "Knowledge", 7, 6, false }, { "Abilities", 13, 6, false } };
    for (int c = 0; c < 2; c++) {
        float x0 = c ? 259.5f : 201, x1 = c ? 317 : 257.5f, y = 21.6f;
        for (const Group &g : c ? COL2 : COL1) {
            P.text(fo, 4.3f, x0 + 1, y + 1.6f, soft, g.name);
            y += ROWH * 0.85f;
            for (int k = 0; k < g.n; k++) {
                int idx = g.first + k;
                bool h = P.over(x0, y, x1, y + ROWH);
                if (h) { P.rect(x0, y, x1, y + ROWH, IM_COL32(226, 204, 160, 255), 1); new_what = g.attr ? 5 : 2; new_which = idx; }
                ImVec2 ic = NsDraw::w(x0 + 3.2f, y + ROWH * 0.5f);
                if (g.attr) ui_attr_icon(P.dl, ic, 4.6f * P.k, idx); else ui_skill_icon(P.dl, ic, 4.6f * P.k, idx);
                const char *name = g.attr ? gm_attr_name(idx) : gm_skill_name(idx);
                char v[24];
                ImU32 vc = ink;
                if (g.attr) {
                    int cur = s.attr[idx], mx = s.attr_max[idx];
                    if (idx == 0 || idx == 1 || idx == 6) { snprintf(v, sizeof v, "%d/%d", cur, mx); if (cur < mx) vc = NsDraw::RED; }
                    else if (cur < mx) { snprintf(v, sizeof v, "%d/%d", cur, mx); vc = NsDraw::RED; }
                    else snprintf(v, sizeof v, "%d", cur);
                } else snprintf(v, sizeof v, "%d", s.skill[idx]);
                if (raised_by(s.who, g.attr ? 0x48 : 0x49, idx, nullptr) > 0) vc = RAISED;   /* (a saint's help, for a while) */
                float vw = P.width(fo, 4.8f, v);
                P.text(fo, 4.8f, x0 + 7, y + 1.4f, ink, name, -1, x1 - vw - 4);
                P.text(fo, 4.8f, 0, y + 1.4f, vc, v, x1 - 1.2f);
                if (party_best(s, g.attr, idx) == 1)          /* the party's best at it: a gold dot */
                    P.dl->AddCircleFilled(NsDraw::w(x1 - 1.2f - vw - 1.8f, y + ROWH * 0.5f), 0.95f * P.k, BEST);
                y += ROWH;
            }
            y += 1.2f;
        }
    }
}

/* a small outlined tag in a list's row, ending at x_end (the initials' size); returns its width */
/* the first n letters of a name (UTF-8: letters, not bytes, "Ülrich") */
static std::string letters(const std::string &t, int n)
{
    size_t e = 0;
    for (int c = 0; c < n && e < t.size(); c++) { e++; while (e < t.size() && ((unsigned char)t[e] & 0xC0) == 0x80) e++; }
    return t.substr(0, e);
}
static float small_tag(NsDraw &P, float x_end, float ry, float rowh, const char *label)
{
    float w = P.width(P.fo, 4.2f, label) + 2.4f;
    P.frame(x_end - w, ry + 1.3f, x_end, ry + rowh - 1.2f, NsDraw::GREEN, 1.1f);
    P.text(P.fo, 4.2f, x_end - w + 1.2f, ry + 1.7f, NsDraw::GREEN, label);
    return w;
}

static void saints(NsDraw &P, const SheetModel &s)
{
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM;
    int ns = (int)gamedata.saints.size();
    /* the choices: known / all, what they help with */
    float x = 63;
    for (int a = 0; a < 2; a++) {
        const char *l = a ? "All saints" : "Known in the party";
        float w = P.width(fo, 4.4f, l) + 4;
        bool sel = V.saints_all == (a == 1);
        P.rect(x, 22, x + w, 28.5f, sel ? IM_COL32(112, 40, 22, 240) : IM_COL32(222, 204, 168, 255), 1.2f);
        P.text(fo, 4.4f, x + 2, 22.8f, sel ? NsDraw::LIGHT : ink, l);
        if (P.hit(x, 22, x + w, 28.5f)) { V.saints_all = a == 1; V.scroll[TAB_SAINTS] = 0; }
        x += w + 1;
    }
    x += 2;
    P.text(fo, 4.4f, x, 22.8f, soft, "Helps with:"); x += P.width(fo, 4.4f, "Helps with:") + 1.5f;
    float y = 22;
    for (int u = 0; u < NUSES; u++) {
        float w = P.width(fo, 4.4f, USES[u].name) + 4;
        if (x + w > 318) { x = 63; y += 7.3f; }
        bool sel = V.use == u;
        P.rect(x, y, x + w, y + 6.5f, sel ? IM_COL32(40, 70, 112, 240) : IM_COL32(222, 204, 168, 255), 1.2f);
        P.text(fo, 4.4f, x + 2, y + 0.8f, sel ? NsDraw::LIGHT : ink, USES[u].name);
        if (P.hit(x, y, x + w, y + 6.5f)) { V.use = u; V.scroll[TAB_SAINTS] = 0; }
        x += w + 1;
    }
    float LY0 = y + 8.5f, LX0 = 62, LX1 = 168, ROWH = 7.6f;
    int NROWS = (int)((197 - LY0) / ROWH);
    std::vector<int> rows;
    for (int t = 0; t < ns; t++) {
        bool known = false; for (int m = 0; m < 5; m++) if (s.present[m] && s.saint[m][t]) known = true;
        if (!V.saints_all && !known) continue;
        if (!saint_fits(t, V.use)) continue;
        rows.push_back(t);
    }
    int n = (int)rows.size();
    int &sc = V.scroll[TAB_SAINTS];
    if (ns_in.wheel != 0 && ns_in.x < LX1) { sc -= (int)(ns_in.wheel * 2); ns_in.wheel = 0; }
    int maxs = n - NROWS; if (maxs < 0) maxs = 0;
    if (sc > maxs) sc = maxs;
    if (sc < 0) sc = 0;
    P.rect(LX0, LY0 - 0.5f, LX1, LY0 + NROWS * ROWH + 0.5f, IM_COL32(250, 242, 222, 255), 1.5f);
    if (!n) P.para(fo, NsDraw::FS_SMALL, LX0 + 3, LY0 + 2, LX1 - LX0 - 6, dim, V.saints_all ? "No saint helps with that." : "Nobody in the party knows a saint that helps with that. (All saints shows the others too.)");
    int hover = -1;
    int cl = (s16)g_rw(G_CUR_LOC);
    const Library *here = cl >= 0 && cl < G_NLOC ? library_at(cl) : nullptr;   /* (the library of the place the party is in: what it teaches, tagged) */
    for (int r = 0; r < NROWS && sc + r < n; r++) {
        int t = rows[sc + r];
        float ry = LY0 + r * ROWH;
        bool h = P.over(LX0, ry, LX1, ry + ROWH);
        if (h) hover = t;
        if (h || t == V.pin_saint) P.rect(LX0 + 0.5f, ry, LX1 - 0.5f, ry + ROWH, t == V.pin_saint ? IM_COL32(218, 196, 150, 255) : IM_COL32(226, 204, 160, 255));
        if (P.hit(LX0, ry, LX1, ry + ROWH)) V.pin_saint = V.pin_saint == t ? -1 : t;
        /* who knows it: their initials, right */
        float ix = LX1 - 2;
        for (int m = 4; m >= 0; m--) {
            if (!s.present[m] || !s.saint[m][t]) continue;
            std::string ini = letters(ns_gtext(s.names[m]), 1);
            float iw = P.width(fo, 4.2f, ini) + 2.4f;
            bool lowv = t < s.nsaints && s.saint_vmin[t] > 0 && s.pskill[m][9] < s.saint_vmin[t];   /* (Virtue below what the saint asks: grey) */
            P.rect(ix - iw, ry + 1.3f, ix, ry + ROWH - 1.2f, lowv ? IM_COL32(150, 146, 140, 200) : m == s.who ? IM_COL32(112, 40, 22, 230) : IM_COL32(120, 84, 40, 200), 1.1f);
            P.text(fo, 4.2f, ix - iw + 1.2f, ry + 1.7f, NsDraw::LIGHT, ini);
            ix -= iw + 0.8f;
        }
        bool known = false; for (int m = 0; m < 5; m++) if (s.present[m] && s.saint[m][t]) known = true;
        if (here && !known) for (int k = 0; k < 4; k++) if (here->saint[k] == t) {
            ix -= small_tag(P, ix, ry, ROWH, "taught here") + 0.8f; break;
        }
        bool any_can = false, knowers = false;                   /* nobody who knows it has the Virtue it asks: slate grey */
        for (int m = 0; m < 5; m++) if (s.present[m] && s.saint[m][t]) { knowers = true; if (!(t < s.nsaints && s.saint_vmin[t] > 0 && s.pskill[m][9] < s.saint_vmin[t])) any_can = true; }
        P.text(f, 5.4f, LX0 + 2.5f, ry + 0.9f, knowers && !any_can ? IM_COL32(110, 116, 128, 255) : ink, gamedata.saints[t].name, -1, ix - 1);
    }
    if (n > NROWS) {
        float LY1 = LY0 + NROWS * ROWH, th = (LY1 - LY0) * NROWS / n, ty = LY0 + (LY1 - LY0 - th) * sc / (float)(n - NROWS);
        P.rect(LX1 - 1.4f, ty, LX1 - 0.3f, ty + th, IM_COL32(120, 80, 36, 140), 0.5f);
    }
    /* the saint: the statue, the name, who knows it (or where it is taught), what the help does
       (mods/saintinfo.h) and where the text says otherwise, whether encounters call on it, the legend */
    int t = hover >= 0 ? hover : V.pin_saint >= 0 ? V.pin_saint : n ? rows[sc] : -1;
    const float DX0 = 171, DX1 = 318, DY0 = LY0 - 0.5f, DY1 = 197;
    P.rect(DX0, DY0, DX1, DY1, IM_COL32(236, 224, 196, 255), 1.5f);
    if (t < 0 || t >= ns) return;
    std::string who;
    for (int m = 0; m < 5; m++) if (s.present[m] && s.saint[m][t]) who += (who.empty() ? "" : ", ") + ns_gtext(s.names[m]);
    std::string where;                                   /* nobody knows it: the libraries that teach it (of the places the party knows: ui/libraries.h) */
    if (who.empty()) {
        std::vector<LibraryHit> lh = libraries_teaching(t);
        if (lh.empty()) where = "No library the party has been to teaches this saint.";
        else {
            where = "Taught in the library of ";
            size_t k = std::min(lh.size(), (size_t)3);
            for (size_t i = 0; i < k; i++) {
                int d = (int)(lh[i].days + 0.5);
                std::string ds = lh[i].days < 0 ? "" : lh[i].days < 0.6 ? " (here)" : " (" + std::to_string(d) + (d == 1 ? " day)" : " days)");
                where += (i ? (i + 1 == k && lh.size() <= 3 ? " and " : ", ") : "") + lh[i].lib->name + ds;
            }
            if (lh.size() > 3) where += " and " + std::to_string(lh.size() - 3) + " more";
            where += ".";
        }
    }
    const std::string &lore = gamedata.saints[t].text;
    float lore_h = P.para_height(fo, 4.4f, DX1 - DX0 - 5, lore) + 6;   /* (and the note under it) */
    const GamePic *g = t < s.nsaints ? statue(s.saint_class[t]) : nullptr;
    float tx = DX0 + 2, sh = fmaxf(56, fminf(116, DY1 - DY0 - 4 - lore_h));   /* the statue smaller for a long legend */
    if (g) {
        float sw = g->w * sh / g->h;                        /* (the statue in its niche) */
        P.dl->AddImageRounded((ImTextureID)(intptr_t)g->tex, NsDraw::w(DX0 + 2, DY0 + 1.5f), NsDraw::w(DX0 + 2 + sw, DY0 + 1.5f + sh), ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE, 1.5f * P.k);
        tx = DX0 + 4 + sw;
    }
    P.dl->PushClipRect(NsDraw::w(DX0, DY0), NsDraw::w(DX1, DY1), true);
    float ty = DY0 + 2, tw = DX1 - tx - 2;
    ty += P.para(f, NsDraw::FS_HEAD, tx, ty, tw, ink, gamedata.saints[t].name) + 1.5f;
    ty += P.para(fo, NsDraw::FS_SMALL, tx, ty, tw, who.empty() ? NsDraw::RED : soft, who.empty() ? "Nobody in the party knows this saint." : "Known by " + who + ".") + 1;
    if (t < s.nsaints && s.saint_vmin[t] > 0) {                /* the Virtue it asks; who lacks it (Virtue rises slowly: nothing restores it) */
        std::string low;
        for (int m = 0; m < 5; m++) if (s.present[m] && s.saint[m][t] && s.pskill[m][9] < s.saint_vmin[t]) low += (low.empty() ? "" : ", ") + ns_gtext(s.names[m]) + " (" + std::to_string(s.pskill[m][9]) + ")";
        char b[200]; snprintf(b, sizeof b, low.empty() ? "Asks a Virtue of %d." : "Asks a Virtue of %d: too low for %s, who cannot be heard.", s.saint_vmin[t], low.c_str());
        ty += P.para(fo, 4.4f, tx, ty, tw, low.empty() ? soft : IM_COL32(90, 98, 112, 255), b) + 1;
    }
    if (!where.empty()) ty += P.para(fo, 4.4f, tx, ty, tw, soft, where) + 1.5f;
    else {                                               /* pray: a button for each who knows it (the original: a click on the saint) */
        float bx = tx, bh = 8.5f;
        for (int m = 0; m < 5; m++) {
            if (!s.present[m] || !s.saint[m][t]) continue;
            std::string lb = ns_gtext(s.names[m]) + " prays";
            float bw = P.width(P.fo, NsDraw::FS_SMALL, lb) + 6;
            if (bx + bw > DX1 - 1 && bx > tx) { bx = tx; ty += bh + 1; }
            if (P.button(bx, ty, bx + bw, ty + bh, lb, IM_COL32(40, 70, 130, 235))) ns_push(SV_PRAY, m, t, 0);
            if (P.over(bx, ty, bx + bw, ty + bh)) ns_tip("Call on this saint now: the prayer screen, where you choose how much divine favor to spend.", 20);
            bx += bw + 1.5f;
        }
        ty += bh + 2;
    }
    ty += P.para(fo, NsDraw::FS_SMALL, tx, ty, tw, IM_COL32(26, 64, 132, 255), saint_info_does(t)) + 1.5f;
    const char *un = saint_info_unlike(t);
    if (*un) ty += P.para(fo, 4.4f, tx, ty, tw, IM_COL32(140, 50, 30, 255), std::string("Differs from the description: ") + un) + 1.5f;
    ty += P.para(fo, 4.2f, tx, ty, tw, ns_tier_colour(saint_info_cards(t) >= 10 ? 20 : saint_info_cards(t) >= 3 ? 5 : saint_info_cards(t)), saint_encounters_line(t)) + 1.5f;
    float ly = fmaxf(ty, g ? DY0 + 1.5f + sh + 2 : ty);
    ly += P.para(fo, 4.4f, DX0 + 2.5f, ly, DX1 - DX0 - 5, soft, lore) + 1.5f;
    if (*saint_info_note(t)) P.para(fo, 4.0f, DX0 + 2.5f, fmaxf(ly, DY1 - 5.5f), DX1 - DX0 - 5, dim, saint_info_note(t));   /* (at the foot) */
    P.dl->PopClipRect();
}

/* the ingredients of formula k the party has enough of, as batches */
static int batches_of(const SheetModel &s, int k)
{
    if (k < 0 || k >= (int)gamedata.formulas.size()) return 0;
    int b = INT_MAX;
    for (const FormulaIngr &g : gamedata.formulas[k].ingr) {
        int have = g.item >= 0 && g.item < 200 ? s.have[g.item] : 0;
        if (g.count > 0) b = std::min(b, have / g.count);
    }
    return b == INT_MAX ? 0 : b;
}

/* an ingredient on the shelf: what the party has of it, who carries it, where it is sold, what uses it */
static void ingredient_tip(const SheetModel &s, int id)
{
    ns_tip_begin(22);
    ImGui::TextUnformatted(ns_item_name(id).c_str());
    int n = s.have[id];
    std::string by;
    for (int m = 0; m < 5; m++) if (s.present[m] && s.ingr[m][id - 72]) by += (by.empty() ? "" : ", ") + ns_gtext(s.names[m]) + " " + std::to_string(s.ingr[m][id - 72]);
    if (!n) ImGui::TextDisabled("The party has none.");
    else if (s.shared) ImGui::Text("The party has %d.", n);
    else ImGui::Text("The party has %d (%s).", n, by.c_str());
    std::string w = journal_where_sold(id);
    if (!w.empty()) ImGui::TextDisabled("Where to buy: %s", w.c_str());
    std::string uses;                                   /* the formulae the party knows that call for it */
    int nu = 0;
    for (int k = 0; k < (int)gamedata.formulas.size() && k < 66; k++) {
        bool kn = false; for (int m = 0; m < 5; m++) if (s.present[m] && s.formula[m][k]) kn = true;
        if (!kn) continue;
        for (const FormulaIngr &g : gamedata.formulas[k].ingr) if (g.item == id) { if (nu < 4) uses += (uses.empty() ? "" : ", ") + gamedata.formula_names[k] + " (" + std::to_string(g.count) + ")"; nu++; }
    }
    if (nu > 4) uses += " and " + std::to_string(nu - 4) + " more";
    if (!uses.empty()) ImGui::TextWrapped("Used in %s.", uses.c_str());
    ns_tip_end();
}

static void formulae(NsDraw &P, const SheetModel &s)
{
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM;
    int nf = (int)std::min(std::min(gamedata.formula_names.size(), gamedata.formulas.size()), (size_t)66);
    /* the alchemists: everyone who knows a formula */
    std::vector<int> al;
    for (int m = 0; m < 5; m++) { if (!s.present[m]) continue; for (int k = 0; k < nf; k++) if (s.formula[m][k]) { al.push_back(m); break; } }
    auto known = [&](int k) { for (int m : al) if (s.formula[m][k]) return true; return false; };
    const float LX0 = 62, LX1 = 200, LY0 = 30.5f, ROWH = 7.6f; const int NROWS = 14;
    /* the choice: the formulae the party knows, or all of them (to judge one before buying it) */
    float x = LX0 + 1;
    for (int a = 0; a < 2; a++) {
        const char *l = a ? "All formulae" : "Known in the party";
        float w = P.width(fo, 4.4f, l) + 4;
        bool sel = V.formulae_all == (a == 1);
        P.rect(x, 22, x + w, 28.5f, sel ? IM_COL32(112, 40, 22, 240) : IM_COL32(222, 204, 168, 255), 1.2f);
        P.text(fo, 4.4f, x + 2, 22.8f, sel ? NsDraw::LIGHT : ink, l);
        if (P.hit(x, 22, x + w, 28.5f)) { V.formulae_all = a == 1; V.scroll[TAB_FORMULAE] = 0; }
        x += w + 1;
    }
    /* who knows each: a column a member, headed by the name; its first three letters when the
       names would take too much of the row ("Han" read as a word of its own) */
    float colw = 9.5f; bool full = true;
    for (int m : al) colw = std::max(colw, P.width(fo, 4.2f, ns_gtext(s.names[m])) + 2.5f);
    if (colw * al.size() > 44) { colw = 9.5f; full = false; }
    float cx0 = LX1 - 2 - colw * al.size();
    for (size_t a = 0; a < al.size(); a++) {
        std::string ini = full ? ns_gtext(s.names[al[a]]) : letters(ns_gtext(s.names[al[a]]), 3);
        P.text(fo, 4.2f, cx0 + colw * a + (colw - P.width(fo, 4.2f, ini)) * 0.5f, 24.2f, al[a] == s.who ? NsDraw::RED : ink, ini);
    }
    std::vector<int> rows;
    for (int k = 0; k < nf; k++) if (V.formulae_all || known(k)) rows.push_back(k);
    int n = (int)rows.size();
    int &sc = V.scroll[TAB_FORMULAE];
    if (ns_in.wheel != 0 && ns_in.x < LX1 && ns_in.y < LY0 + NROWS * ROWH) { sc -= (int)(ns_in.wheel * 2); ns_in.wheel = 0; }
    int maxs = n - NROWS; if (maxs < 0) maxs = 0;
    if (sc > maxs) sc = maxs;
    if (sc < 0) sc = 0;
    P.rect(LX0, LY0 - 0.5f, LX1, LY0 + NROWS * ROWH + 0.5f, IM_COL32(250, 242, 222, 255), 1.5f);
    if (!n) P.para(fo, NsDraw::FS_SMALL, LX0 + 3, LY0 + 2, LX1 - LX0 - 6, dim, "Nobody in the party knows a formula. All formulae shows every one, its ingredients and where it is sold.");
    int hover = -1;
    int cl = (s16)g_rw(G_CUR_LOC), sold[4] = { -1, -1, -1, -1 }, price[4];
    if (!(cl >= 0 && cl < (int)gamedata.cities.size() && alchemist_formulae(cl, sold, price))) sold[0] = sold[1] = sold[2] = sold[3] = -1;   /* (the town's alchemist: what it sells, tagged) */
    for (int r = 0; r < NROWS && sc + r < n; r++) {
        int k = rows[sc + r];
        float ry = LY0 + r * ROWH;
        bool h = P.over(LX0, ry, LX1, ry + ROWH), kn = known(k);
        if (h) hover = k;
        if (h || k == V.pin_formula) P.rect(LX0 + 0.5f, ry, LX1 - 0.5f, ry + ROWH, k == V.pin_formula ? IM_COL32(218, 196, 150, 255) : IM_COL32(226, 204, 160, 255));
        if (P.hit(LX0, ry, LX1, ry + ROWH)) V.pin_formula = V.pin_formula == k ? -1 : k;
        P.icon(LX0 + 4.5f, ry + ROWH * 0.5f, 5.8f, 95 + k / 3, -1, k % 3 + 1);
        float end = cx0 - 1;
        int b = kn ? batches_of(s, k) : 0;
        if (b) {                                         /* what can be brewed now: how many batches */
            std::string bs = std::to_string(b) + "x";
            P.text(fo, NsDraw::FS_SMALL, 0, ry + 1.5f, NsDraw::GREEN, bs, end);
            end -= P.width(fo, NsDraw::FS_SMALL, bs) + 1.5f;
        }
        if (!kn && (sold[0] == k || sold[1] == k || sold[2] == k || sold[3] == k)) {
            end -= small_tag(P, end, ry, ROWH, "sold here") + 1;
        }
        P.text(f, 5.4f, LX0 + 9, ry + 0.9f, kn ? ink : dim, gamedata.formula_names[k], -1, end);
        for (size_t a = 0; a < al.size(); a++) {
            if (!s.formula[al[a]][k]) continue;
            ImVec2 c = NsDraw::w(cx0 + colw * a + colw * 0.5f, ry + ROWH * 0.5f);
            P.dl->AddCircleFilled(c, 1.3f * P.k, al[a] == s.who ? NsDraw::RED : IM_COL32(120, 84, 40, 255));
        }
    }
    if (n > NROWS) {
        float LY1 = LY0 + NROWS * ROWH, th = (LY1 - LY0) * NROWS / n, ty = LY0 + (LY1 - LY0 - th) * sc / (float)(n - NROWS);
        P.rect(LX1 - 1.4f, ty, LX1 - 0.3f, ty + th, IM_COL32(120, 80, 36, 140), 0.5f);
    }
    int k = hover >= 0 ? hover : V.pin_formula >= 0 ? V.pin_formula : n ? rows[sc] : -1;
    if (k >= nf) k = -1;

    /* the party's ingredients: all 23 always in the same places, those it has none of faded; what
       the formula shown calls for ringed, green when there is enough */
    const float SY0 = LY0 + NROWS * ROWH + 2.5f, GY0 = SY0 + 6, cellw = (LX1 - LX0 - 2) / 8.0f, cellh = (197 - GY0) / 3.0f;
    P.text(fo, 4.3f, LX0 + 2, SY0, soft, "The party's ingredients");
    int tip = -1;
    for (int i = 0; i < 23; i++) {
        int id = 72 + i, have = s.have[id], need = 0;
        if (k >= 0) for (const FormulaIngr &g : gamedata.formulas[k].ingr) if (g.item == id) need += g.count;
        float x0 = LX0 + 1 + (i % 8) * cellw, y0 = GY0 + (i / 8) * cellh, cxm = x0 + cellw * 0.5f;
        if (need) P.rect(x0 + 0.3f, y0 + 0.3f, x0 + cellw - 0.3f, y0 + cellh - 0.3f, have >= need ? IM_COL32(204, 226, 184, 255) : IM_COL32(238, 196, 180, 255), 1.5f);
        if (P.over(x0, y0, x0 + cellw, y0 + cellh)) { tip = id; P.rect(x0 + 0.3f, y0 + 0.3f, x0 + cellw - 0.3f, y0 + cellh - 0.3f, IM_COL32(226, 204, 160, 160), 1.5f); }
        float is = std::min(cellw * 0.5f, cellh * 0.46f);         /* (a picture is drawn up to a third wider than its size) */
        float iy = y0 + cellh * 0.36f;
        P.icon(cxm, iy, is, id);
        if (!have) P.dl->AddCircleFilled(NsDraw::w(cxm, iy), is * 0.7f * P.k, need ? IM_COL32(238, 196, 180, 160) : IM_COL32(242, 230, 200, 165));   /* (none: the sign faded) */
        std::string c = need ? std::to_string(have) + "/" + std::to_string(need) : have ? std::to_string(have) : "";   /* (none: the faded sign says it) */
        ImU32 cc = need ? (have >= need ? NsDraw::GREEN : NsDraw::RED) : ink;
        if (!c.empty()) P.text(fo, NsDraw::FS_SMALL, cxm - P.width(fo, NsDraw::FS_SMALL, c) * 0.5f, y0 + cellh * 0.68f, cc, c);
    }
    if (tip >= 0) ingredient_tip(s, tip);

    /* the formula: what it does, how hard and risky, its ingredients, the batches; who can brew
       it and their chance, or (nobody knows it) where it is sold */
    const float DX0 = 203, DX1 = 318, DY1 = 197, DW = DX1 - DX0 - 5;
    P.rect(DX0, 22, DX1, DY1, IM_COL32(236, 224, 196, 255), 1.5f);
    if (k < 0) return;
    const Formula &fm = gamedata.formulas[k];
    bool kn = known(k);
    P.dl->PushClipRect(NsDraw::w(DX0, 22), NsDraw::w(DX1, DY1), true);
    float y = 24;
    P.icon(DX0 + 6, y + 4, 8, 95 + k / 3, -1, k % 3 + 1);
    y += P.para(f, NsDraw::FS_HEAD, DX0 + 12, y, DW - 9, ink, gamedata.formula_names[k]) + 1.5f;
    static const char *RISK[3] = { "low risk", "some risk", "high risk" };
    char mr[64]; snprintf(mr, sizeof mr, "Mystic number %d, %s.", fm.mystic, fm.risk >= 0 && fm.risk < 3 ? RISK[fm.risk] : "risk unknown");
    float mh = P.para(fo, NsDraw::FS_SMALL, DX0 + 2.5f, y, DW, soft, mr);
    if (P.over(DX0, y, DX1, y + mh))
        ns_tip("The mystic number: the higher, the harder the formula is to brew (set against the alchemist's Alchemy and Intelligence). The risk: how badly a failed brew can go wrong.");
    y += mh + 1;
    y += P.para(fo, NsDraw::FS_SMALL, DX0 + 2.5f, y, DW, IM_COL32(26, 64, 132, 255), game_text(fm.text.c_str(), fm.text.size())) + 0.5f;
    { ImU32 ec; std::string enc = ns_item_encounters(95 + k / 3, &ec); if (!enc.empty()) y += P.para(fo, NsDraw::FS_SMALL, DX0 + 2.5f, y, DW, ec, enc); }
    y += 1.5f;
    P.text(fo, 4.3f, DX0 + 2.5f, y, soft, "Ingredients: the party has / needed"); y += 6;
    for (const FormulaIngr &g : fm.ingr) {
        int have = g.item >= 0 && g.item < 200 ? s.have[g.item] : 0;
        P.icon(DX0 + 5, y + 3, 5.5f, g.item);
        P.text(fo, NsDraw::FS_SMALL, DX0 + 9.5f, y + 0.6f, ink, ns_item_name(g.item), -1, DX1 - 20);
        char v[16]; snprintf(v, sizeof v, "%d / %d", have, g.count);
        P.text(fo, NsDraw::FS_SMALL, 0, y + 0.6f, have >= g.count ? NsDraw::GREEN : NsDraw::RED, v, DX1 - 3);
        y += 6.6f;
    }
    int b = batches_of(s, k);
    y += 0.5f;
    if (b) { char bl[48]; snprintf(bl, sizeof bl, "Enough for %d batch%s.", b, b == 1 ? "" : "es"); y += P.para(fo, NsDraw::FS_SMALL, DX0 + 2.5f, y, DW, NsDraw::GREEN, bl) + 1.5f; }
    else y += P.para(fo, NsDraw::FS_SMALL, DX0 + 2.5f, y, DW, dim, kn ? "The party lacks an ingredient: brewing must wait." : "The party lacks an ingredient.") + 1.5f;
    if (kn) {
        P.text(fo, 4.3f, DX0 + 2.5f, y, soft, "Chance of brewing one batch"); y += 6;
        for (int m : al) {
            if (!s.formula[m][k]) continue;
            int pc = s.intel[m] + (s.alch[m] - fm.mystic) + 100;  /* (the alchemy screen's, without the Philosopher's Stone) */
            pc = pc < 0 ? 0 : pc > 99 ? 99 : pc;
            char v[64]; snprintf(v, sizeof v, "%s: %d%%", ns_gtext(s.names[m]).c_str(), pc);
            P.text(fo, NsDraw::FS_SMALL, DX0 + 4, y, ink, v); y += 6;
        }
    } else {                                             /* nobody knows it: where an alchemist sells it (of the towns the party knows: ui/libraries.h) */
        y += P.para(fo, NsDraw::FS_SMALL, DX0 + 2.5f, y, DW, NsDraw::RED, "Nobody in the party knows it.") + 1;
        std::vector<SellerHit> sh = formula_sellers(k);
        std::string w;
        if (k >= 22) w = "No alchemist sells it: it is traded for or found.";
        else if (sh.empty()) w = "No alchemist the party has been to sells it.";
        else {
            int d = (int)(sh[0].days + 0.5);
            bool here = sh[0].days >= 0 && sh[0].days < 0.6;
            w = here ? "Sold here, " + std::to_string(sh[0].price_fl) + " florins."
                     : "Sold by the alchemist of " + sh[0].name + (sh[0].days < 0 ? "" : ", " + std::to_string(d) + (d == 1 ? " day away" : " days away")) + ", " + std::to_string(sh[0].price_fl) + " florins.";
            if (sh.size() > 1) {
                w += " Also in ";
                for (size_t i = 1; i < sh.size() && i < 4; i++) w += (i > 1 ? ", " : "") + sh[i].name;
                if (sh.size() > 4) w += " and " + std::to_string(sh.size() - 4) + " more";
                w += ".";
            }
        }
        P.para(fo, NsDraw::FS_SMALL, DX0 + 2.5f, y, DW, soft, w);
    }
    P.dl->PopClipRect();
}

extern "C" void sheetview_draw(void)
{
    const SheetModel &s = sheet_model;
    partypanel_focus = -1;
    if (!sheetview_showing()) { if (sheetview_dragging) { sheetview_dragging = 0; V.dragging = false; } return; }
    NsDraw P;
    if (!P.begin()) return;
    partypanel_focus = s.who;
    if (s.who != V.last_who) { V.scroll[TAB_THINGS] = 0; V.drop_row = -1; V.what = 0; V.last_who = s.who; }
    if (V.act_seq != s.seq) V.drop_row = -1;
    int new_what = 0, new_which = -1;
    EFF = effects_now(5);
    P.page();
    header(P, s);
    if (V.tab == TAB_THINGS || V.tab == TAB_SKILLS) {
        doll(P, s);
        if (V.tab == TAB_THINGS) things(P, s, new_what, new_which);
        else skills(P, s, new_what, new_which);
        if (new_what) { V.what = new_what; V.which = new_which; V.seq = s.seq; }
        else if (V.seq != s.seq) V.what = 0;
        else if (!P.over(200, 21, 318, 197)) V.what = 0;
        strip(P, s, 200, 318, V.tab == TAB_THINGS ? 159.5f : 152, 197);
    } else if (V.tab == TAB_SAINTS) saints(P, s);
    else formulae(P, s);
}

/* for the prayer screen (ui/prayview.cpp): the saint's statue, its legend and effect */
const GamePic *ns_saint_statue(int cls) { return statue(cls); }

extern "C" int sheetview_pointed_item(void) { return V.what == 1 ? V.which : -1; }
