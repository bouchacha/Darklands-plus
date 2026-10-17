/* sheetview.cpp - draws a member's sheet (mods/sheetview.c runs it; docs/NATIVE_SCREENS.md),
 * the whole page anew beside the party panel.
 *
 * Top: the member's name, sex and age, the leader, the tabs, Close.
 * Things: the paper doll on its column (put together from the game's own pictures, as the
 *   original does: PICS\ARMSBACK.PIC and the armour, weapon, shield and missile weapon over it),
 *   the load under it; right, the things, those in use first, the quality only where it can
 *   differ; Ready / Unequip / Drink and Drop on the row under the pointer, Undo for what was
 *   dropped; a thing dragged onto a member at the left is given to them (Shift: one of a stack).
 * Skills: the doll; right, the attributes and the skills on one page in the original's groups,
 *   each with its sign; Endurance, Strength and Divine Favor as current/maximum, the others
 *   their value (with the maximum, in red, when lowered).
 * Saints: the whole party's - each saint someone knows (or all), who knows it, a choice of what
 *   they help with; the saint under the pointer (or clicked) with the game's statue for it (its
 *   prayer screen's picture), the legend, and what it does in another colour.
 * Formulae: the whole party's - each formula someone knows, who knows it; the one under the
 *   pointer with its effect, mystic number and risk, its ingredients against what the party
 *   carries, and each alchemist's chance with one batch. */
#include "imgui.h"
#include "../runtime/config.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../mods/sheetview.h"
#include "nsdraw.h"
#include "gamepics.h"

extern "C" u32 plat_ticks_ms(void);
extern "C" int partypanel_member_at(int gx, int gy);
extern int partypanel_focus;
void ui_item_icon(ImDrawList *dl, ImVec2 c, float s, int id);
void ui_attr_icon(ImDrawList *dl, ImVec2 c, float s, int k);
void ui_skill_icon(ImDrawList *dl, ImVec2 c, float s, int k);
const char *ui_attr_help(int a);
const char *ui_skill_help(int k);
std::string rules_weapons_of_skill(int k);
size_t saint_effect_at(const std::string &text);

namespace {
const char *ATTR[7] = { "Endurance", "Strength", "Agility", "Perception", "Intellect", "Charisma", "Divine Favor" };
const char *SKILL[19] = { "Edged Weapons", "Impact Weapons", "Flails", "Polearms", "Thrown Weapons", "Bows", "Missile Devices",
    "Alchemy", "Religion", "Virtue", "Speak Common", "Speak Latin", "Read & Write", "Healing", "Artifice", "Stealth",
    "Streetwise", "Riding", "Woodwise" };
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
    bool saints_all = false; int use = 0;
    /* dragging a thing */
    int drag_row = -1; bool dragging = false;
} V;

std::string lower(std::string s) { for (char &c : s) c = (char)tolower((unsigned char)c); return s; }

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

/* a saint's description: legend, then what it does */
void saint_parts(const std::string &t, std::string &lore, std::string &eff)
{
    size_t a = saint_effect_at(t);
    lore = t.substr(0, a); size_t b = a; while (b < t.size() && t[b] == ' ') b++;
    eff = t.substr(b);
    while (!lore.empty() && lore.back() == ' ') lore.pop_back();
}
bool saint_fits(const std::string &t, int use)
{
    if (use <= 0) return true;
    std::string lore, eff; saint_parts(t, lore, eff);
    std::string e = lower(eff.empty() ? t : eff);
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
    if (P.button(296, 2.5f, 318, 10.2f, "Close", IM_COL32(112, 40, 22, 235), NsDraw::FS_SMALL)) ns_push(SV_CLOSE, 0, 0, 0);
    char sub[64]; snprintf(sub, sizeof sub, "%s, age %d", s.female ? "Woman" : "Man", s.age);
    P.text(fo, NsDraw::FS_SMALL, 63.5f, 12.4f, NsDraw::INK_SOFT, sub);
    float lx = 63.5f + P.width(fo, NsDraw::FS_SMALL, sub) + 3;
    if (s.who == s.leader) {
        P.rect(lx, 11.6f, lx + 17, 18.8f, IM_COL32(214, 172, 60, 255), 1.2f);
        P.text(fo, NsDraw::FS_SMALL, lx + 2, 12.4f, NsDraw::INK, "Leader");
    } else if (P.button(lx, 11.6f, lx + 26, 18.8f, "Make leader", IM_COL32(120, 84, 40, 235), 4.4f)) ns_push(SV_LEADER, s.who, 0, 0);
    if (P.over(lx, 11.6f, lx + (s.who == s.leader ? 17 : 26), 18.8f))
        ImGui::SetTooltip(config_get_int("assist.best_leader", 1) && !config_get_int("original", 0)
            ? "A card's tests are made by whoever is best at them and shops price by the best bargainer, whoever leads.\n"
              "The leader still takes what befalls \"the leader\" on a card (a mine's traps, a church's favour, a thing handed over),\n"
              "and in a battle or a dungeon, unpaused, deals with doors, chests, locks and traps, and leads the group's moves."
            : "The leader speaks and acts for the party on the cards and bargains in the shops.");
    static const char *TABS[4] = { "Things", "Skills", "Saints", "Formulae" };
    float tw = 25.5f, x = 318 - 4 * tw;
    for (int t = 0; t < 4; t++) {
        bool sel = t == V.tab, h = P.over(x, 11, x + tw - 0.8f, 19.5f);
        P.rect(x, 11, x + tw - 0.8f, 19.5f, sel ? IM_COL32(112, 40, 22, 240) : h ? IM_COL32(214, 190, 150, 255) : IM_COL32(222, 204, 168, 255), 1.5f);
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
             "Load %d: the weight of what %s has in use (weapons, armour, shield). Spare things in the pack weigh nothing.\n"
             "Against Endurance + Strength (%d now): light up to %d, normal up to %d, heavy up to %d, then overloaded.\n\n"
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
    const GamePic *g = gamepic_layers(doll_layers(s), "PICS\\ARMBACK.PIC");
    if (g) P.dl->AddImage((ImTextureID)(intptr_t)g->tex, NsDraw::w(62, 21.5f), NsDraw::w(62 + g->w, 21.5f + g->h));
    else P.rect(62, 21.5f, 198, 191.5f, IM_COL32(200, 196, 190, 255));
    /* the load */
    int band = s.load_band < 0 ? 0 : s.load_band > 3 ? 3 : s.load_band;
    ImU32 c = band >= 3 ? NsDraw::RED : band == 2 ? IM_COL32(150, 80, 20, 255) : NsDraw::INK;
    P.text(P.fo, NsDraw::FS_SMALL, 63.5f, 192.3f, c, LOAD[band]);
    char v[24]; snprintf(v, sizeof v, "%d of %d", s.load, s.capacity);
    P.text(P.fo, NsDraw::FS_SMALL, 0, 192.3f, NsDraw::INK_SOFT, v, 197);
    float f = s.capacity > 0 ? fminf(1.5f, (float)s.load / s.capacity) / 1.5f : 0;
    P.rect(100, 194.6f, 160, 196.4f, IM_COL32(214, 196, 160, 255), 0.6f);
    P.rect(100, 194.6f, 100 + 60 * f, 196.4f, c, 0.6f);
    P.rect(100 + 60 / 3.0f, 194.2f, 100 + 60 / 3.0f + 0.4f, 196.8f, IM_COL32(120, 80, 36, 160));
    P.rect(100 + 120 / 3.0f, 194.2f, 100 + 120 / 3.0f + 0.4f, 196.8f, IM_COL32(120, 80, 36, 160));
    if (P.over(62, 191.5f, 198, 198) && config_get_int("assist.shop_info", 1)) {   /* what the load does */
        ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(ImGui::GetFontSize() * 26, FLT_MAX));
        ImGui::BeginTooltip(); ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25);
        ImGui::TextUnformatted(load_effects(s).c_str());
        ImGui::PopTextWrapPos(); ImGui::EndTooltip();
    }
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
        std::string l1 = std::string(d ? kind_name(d->kind) : "");
        if (shows_quality(d, it.qual)) l1 += ns_quality_words(it.id, it.qual);
        l1 += ", weight " + std::to_string(it.weight);
        if (it.count > 1) l1 += " each, " + std::to_string(it.count) + " of them";
        if (it.equipped == 2) l1 += "; this member's mount";
        else if (it.equipped) l1 += "; in use";
        else if (it.holder < 0) l1 += it.count > 1 ? " in the party's pack" : "; in the party's pack";
        if (it.kept) l1 += "; kept (not sold or dropped)";
        y += P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y, SW, soft, l1) + 0.5f;
        std::string l2 = ns_item_line(it.id);
        if (!l2.empty()) y += P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y, SW, soft, l2) + 0.5f;
        std::string use = ns_item_use(it.id);
        if (!use.empty()) y += P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y, SW, soft, use) + 0.5f;
        if (d && d->type >= 0 && d->type <= 33 && (d->kind == K_WEAPON || d->kind == K_MISSILE)) {
            int sk = rb(G_FAR20A5, (u16)(0x75ac + d->type)), minsk = rb(G_FAR20A5, (u16)(0x7726 + d->type)), minst = rb(G_FAR20A5, (u16)(0x7765 + d->type));
            int have = sk < 7 ? s.skill[sk] : 0;
            std::string fit = (have < minsk ? "lacks " + std::to_string(minsk - have) + " skill" : std::string("")) +
                              (s.attr[1] < minst ? std::string(have < minsk ? ", " : "") + "too weak by " + std::to_string(minst - s.attr[1]) : "");
            P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y, SW, fit.empty() ? NsDraw::GREEN : NsDraw::RED, fit.empty() ? ns_gtext(s.names[s.who]) + " can use it well" : ns_gtext(s.names[s.who]) + ": " + fit);
        } else if (l2.empty() && it.drinkable && it.id >= 95 && 3 * (it.id - 95) < (int)gamedata.formulas.size())
            P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y, SW, soft, game_text(gamedata.formulas[3 * (it.id - 95)].text.c_str(), gamedata.formulas[3 * (it.id - 95)].text.size()));
    } else if (V.what == 2 && V.which >= 0 && V.which < 19) {
        std::string help = ui_skill_help(V.which);
        if (V.which < 7) { std::string w = rules_weapons_of_skill(V.which); help = "Attack and defence with " + (w.empty() ? std::string("these weapons") : w) + "."; }
        y += P.para(f, NsDraw::FS_NAME, SX0 + 2.5f, y, SW, ink, SKILL[V.which]) + 0.5f;
        P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y, SW, soft, help);
    } else if (V.what == 5 && V.which >= 0 && V.which < 7) {
        y += P.para(f, NsDraw::FS_NAME, SX0 + 2.5f, y, SW, ink, ATTR[V.which]) + 0.5f;
        P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y, SW, soft, ui_attr_help(V.which));
    } else if (V.tab == TAB_THINGS && s.shared)
        P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y + 1, SW, dim, "Point at a thing to see it here. The pack is the whole party's: Ready hands the thing to this member first. Potions are each member's own (drag one onto someone at the left to give it). Keep marks a thing never to be sold or dropped. Only what is in use weighs.");
    else if (V.tab == TAB_THINGS)
        P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y + 1, SW, dim, "Point at a thing to see it here. Drag a thing onto someone at the left to give it to them (Shift: just one of a stack). Another member at the left shows their sheet; Esc or Close goes back.");
    else P.para(fo, NsDraw::FS_SMALL, SX0 + 2.5f, y + 1, SW, dim, "Point at an attribute or a skill to see what it does.");
    if (s.note[0] && plat_ticks_ms() - s.note_t < 6000) {
        P.rect(SX0 + 1, SY1 - 8.6f, SX1 - 1, SY1 - 0.6f, IM_COL32(230, 214, 178, 255));
        P.text(fo, NsDraw::FS_SMALL, SX0 + 2.5f, SY1 - 7.8f, IM_COL32(112, 40, 22, 255), ns_gtext(s.note), -1, SX1 - 1);
    }
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
    if (nin) { rows.push_back(-1); for (int i = 0; i < s.nitems; i++) if (s.item[i].equipped) rows.push_back(i); }
    if (npot) { rows.push_back(-3); for (int i = 0; i < s.nitems; i++) if (!s.item[i].equipped && s.item[i].potion) rows.push_back(i); }
    if (s.nitems > nin + npot) { rows.push_back(-2); for (int i = 0; i < s.nitems; i++) if (!s.item[i].equipped && !s.item[i].potion) rows.push_back(i); }
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
    for (int r = 0; r < NROWS && sc + r < n; r++) {
        int i = rows[sc + r];
        float y = LY0 + r * ROWH;
        if (i < 0) {                                       /* a heading */
            const char *hd = i == -1 ? "In use" : i == -3 ? "Potions" : s.shared ? "The party's pack" : "Carried";
            P.text(fo, NsDraw::FS_SMALL, RX0 + 3, y + 2.2f, soft, hd);
            P.rect(RX0 + 3 + P.width(fo, NsDraw::FS_SMALL, hd) + 2, y + 4.8f, RX1 - 3, y + 5.1f, IM_COL32(120, 80, 36, 90));
            continue;
        }
        const SvItem &it = s.item[i];
        bool h = !V.dragging && P.over(RX0, y, RX1, y + ROWH), busy = i == V.drop_row;
        if (h) { new_what = 1; new_which = i; }
        if (h || busy || (V.dragging && i == V.drag_row)) P.rect(RX0 + 0.5f, y, RX1 - 0.5f, y + ROWH, IM_COL32(226, 204, 160, 255));
        P.icon(RX0 + 5.5f, y + ROWH * 0.5f, 6.3f, it.id, it.qual);
        std::string nm = ns_item_name(it.id);
        if (it.count > 1) nm += " (" + std::to_string(it.count) + ")";
        const ItemDef *d = ns_itemdef(it.id);
        float name_end = (h || busy) ? RX1 - 59 : RX1 - 2;
        if (it.kept) {                                     /* kept from selling and dropping: a small seal at the right */
            float kx = (h || busy) ? RX1 - 61 : RX1 - 4;
            P.dl->AddCircleFilled(NsDraw::w(kx, y + ROWH * 0.5f), 1.5f * P.k, IM_COL32(150, 40, 24, 230));
            name_end -= 4;
        }
        P.text(f, NsDraw::FS_NAME, RX0 + 11, y + 1.0f, ink, nm, -1, name_end);
        if (shows_quality(d, it.qual)) {
            char qs[16]; snprintf(qs, sizeof qs, "q%d", it.qual);
            float qx = RX0 + 11 + P.width(f, NsDraw::FS_NAME, nm) + 2;
            if (qx + P.width(fo, NsDraw::FS_SMALL, qs) < name_end) P.text(fo, NsDraw::FS_SMALL, qx, y + 1.9f, soft, qs);
        }
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
        if (P.over(x, y + 1, x + 18, y + ROWH - 1)) ImGui::SetTooltip("%s", it.kept ? "Kept from selling and dropping (Sell spare gear passes it by). A click: no longer kept."
                                                                               : "Keep it: Darklands+ will not sell it (one by one or with Sell spare gear) or drop it until you click again.");
        x += 19;
        if (it.kept) P.button(x, y + 1, x + 18, y + ROWH - 1, "Drop", IM_COL32(170, 150, 130, 200), 4.4f);   /* (greyed: kept) */
        else if (busy) {
            if (P.button(x, y + 1, x + 18, y + ROWH - 1, P.shift || it.count == 1 ? "Sure?" : "One?", IM_COL32(150, 30, 18, 255), 4.4f)) {
                if (pack) ns_push(SV_PDROP, it.id, it.qual, P.shift ? 255 : 1); else ns_push(SV_DROP, it.idx, P.shift ? 255 : 1, 0);
                V.drop_row = -1;
            }
        } else if (P.button(x, y + 1, x + 18, y + ROWH - 1, "Drop", IM_COL32(120, 44, 24, 235), 4.4f)) { V.drop_row = i; V.act_seq = s.seq; }
    }
    if (n > NROWS) {
        float th = (LY1 - LY0) * NROWS / n, ty = LY0 + (LY1 - LY0 - th) * sc / (float)(n - NROWS);
        P.rect(RX1 - 1.4f, ty, RX1 - 0.3f, ty + th, IM_COL32(120, 80, 36, 140), 0.5f);
    }
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
                           : ns_item_name(it.id) + (to == s.who ? "" : ": onto someone at the left");
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
                const char *name = g.attr ? ATTR[idx] : SKILL[idx];
                char v[24];
                ImU32 vc = ink;
                if (g.attr) {
                    int cur = s.attr[idx], mx = s.attr_max[idx];
                    if (idx == 0 || idx == 1 || idx == 6) { snprintf(v, sizeof v, "%d/%d", cur, mx); if (cur < mx) vc = NsDraw::RED; }
                    else if (cur < mx) { snprintf(v, sizeof v, "%d/%d", cur, mx); vc = NsDraw::RED; }
                    else snprintf(v, sizeof v, "%d", cur);
                } else snprintf(v, sizeof v, "%d", s.skill[idx]);
                float vw = P.width(fo, 4.8f, v);
                P.text(fo, 4.8f, x0 + 7, y + 1.4f, ink, name, -1, x1 - vw - 2);
                P.text(fo, 4.8f, 0, y + 1.4f, vc, v, x1 - 1.2f);
                y += ROWH;
            }
            y += 1.2f;
        }
    }
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
        if (!saint_fits(gamedata.saints[t].text, V.use)) continue;
        rows.push_back(t);
    }
    int n = (int)rows.size();
    int &sc = V.scroll[TAB_SAINTS];
    if (ns_in.wheel != 0 && ns_in.x < LX1) { sc -= (int)(ns_in.wheel * 2); ns_in.wheel = 0; }
    int maxs = n - NROWS; if (maxs < 0) maxs = 0;
    if (sc > maxs) sc = maxs;
    if (sc < 0) sc = 0;
    P.rect(LX0, LY0 - 0.5f, LX1, LY0 + NROWS * ROWH + 0.5f, IM_COL32(250, 242, 222, 255), 1.5f);
    if (!n) P.para(fo, NsDraw::FS_SMALL, LX0 + 3, LY0 + 2, LX1 - LX0 - 6, dim, V.saints_all ? "No saint helps with that." : "Nobody in the party knows a saint that helps with that. (All saints: the others too.)");
    int hover = -1;
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
            std::string ini = ns_gtext(s.names[m]).substr(0, 1);
            float iw = P.width(fo, 4.2f, ini) + 2.4f;
            P.rect(ix - iw, ry + 1.3f, ix, ry + ROWH - 1.2f, m == s.who ? IM_COL32(112, 40, 22, 230) : IM_COL32(120, 84, 40, 200), 1.1f);
            P.text(fo, 4.2f, ix - iw + 1.2f, ry + 1.7f, NsDraw::LIGHT, ini);
            ix -= iw + 0.8f;
        }
        P.text(f, 5.4f, LX0 + 2.5f, ry + 0.9f, ink, gamedata.saints[t].name, -1, ix - 1);
    }
    if (n > NROWS) {
        float LY1 = LY0 + NROWS * ROWH, th = (LY1 - LY0) * NROWS / n, ty = LY0 + (LY1 - LY0 - th) * sc / (float)(n - NROWS);
        P.rect(LX1 - 1.4f, ty, LX1 - 0.3f, ty + th, IM_COL32(120, 80, 36, 140), 0.5f);
    }
    /* the saint: the statue, the name, who knows it, the legend and what the saint does */
    int t = hover >= 0 ? hover : V.pin_saint >= 0 ? V.pin_saint : n ? rows[sc] : -1;
    const float DX0 = 171, DX1 = 318, DY0 = LY0 - 0.5f;
    P.rect(DX0, DY0, DX1, 197, IM_COL32(236, 224, 196, 255), 1.5f);
    if (t < 0 || t >= ns) return;
    const GamePic *g = t < s.nsaints ? statue(s.saint_class[t]) : nullptr;
    float tx = DX0 + 2, sh = fminf(116, 197 - DY0 - 40);
    if (g) {
        float sw = g->w * sh / g->h;                        /* (the statue in its niche) */
        P.dl->AddImageRounded((ImTextureID)(intptr_t)g->tex, NsDraw::w(DX0 + 2, DY0 + 1.5f), NsDraw::w(DX0 + 2 + sw, DY0 + 1.5f + sh), ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE, 1.5f * P.k);
        tx = DX0 + 4 + sw;
    }
    float ty = DY0 + 2, tw = DX1 - tx - 2;
    ty += P.para(f, NsDraw::FS_HEAD, tx, ty, tw, ink, gamedata.saints[t].name) + 1.5f;
    std::string who;
    for (int m = 0; m < 5; m++) if (s.present[m] && s.saint[m][t]) who += (who.empty() ? "" : ", ") + ns_gtext(s.names[m]);
    ty += P.para(fo, NsDraw::FS_SMALL, tx, ty, tw, who.empty() ? NsDraw::RED : soft, who.empty() ? "Nobody in the party knows this saint." : "Known by " + who + ".") + 1.5f;
    std::string lore, eff; saint_parts(gamedata.saints[t].text, lore, eff);
    if (!eff.empty()) ty += P.para(fo, NsDraw::FS_SMALL, tx, ty, tw, IM_COL32(26, 64, 132, 255), eff) + 1.5f;
    float ly = fmaxf(ty, g ? DY0 + 1.5f + sh + 2 : ty);
    if (!lore.empty()) P.para(fo, 4.4f, DX0 + 2.5f, ly, DX1 - DX0 - 5, soft, lore);
}

static void formulae(NsDraw &P, const SheetModel &s)
{
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM;
    int nf = (int)std::min(gamedata.formula_names.size(), (size_t)66);
    /* the alchemists: everyone who knows a formula */
    std::vector<int> al;
    for (int m = 0; m < 5; m++) { if (!s.present[m]) continue; for (int k = 0; k < nf; k++) if (s.formula[m][k]) { al.push_back(m); break; } }
    const float LX0 = 62, LX1 = 200, LY0 = 30.5f, ROWH = 7.6f; const int NROWS = 21;
    float colw = 9.5f, cx0 = LX1 - 2 - colw * al.size();
    P.text(fo, NsDraw::FS_SMALL, LX0 + 2, 22.6f, soft, al.empty() ? "Nobody in the party knows a formula." : "Formulae the party knows; who knows each:");
    for (size_t a = 0; a < al.size(); a++) {
        std::string ini = ns_gtext(s.names[al[a]]).substr(0, 3);
        P.text(fo, 4.2f, cx0 + colw * a + (colw - P.width(fo, 4.2f, ini)) * 0.5f, 24.2f, al[a] == s.who ? NsDraw::RED : ink, ini);
    }
    std::vector<int> rows;
    for (int k = 0; k < nf; k++) for (int m : al) if (s.formula[m][k]) { rows.push_back(k); break; }
    int n = (int)rows.size();
    int &sc = V.scroll[TAB_FORMULAE];
    if (ns_in.wheel != 0 && ns_in.x < LX1) { sc -= (int)(ns_in.wheel * 2); ns_in.wheel = 0; }
    int maxs = n - NROWS; if (maxs < 0) maxs = 0;
    if (sc > maxs) sc = maxs;
    if (sc < 0) sc = 0;
    P.rect(LX0, LY0 - 0.5f, LX1, LY0 + NROWS * ROWH + 0.5f, IM_COL32(250, 242, 222, 255), 1.5f);
    int hover = -1;
    for (int r = 0; r < NROWS && sc + r < n; r++) {
        int k = rows[sc + r];
        float ry = LY0 + r * ROWH;
        bool h = P.over(LX0, ry, LX1, ry + ROWH);
        if (h) hover = k;
        if (h || k == V.pin_formula) P.rect(LX0 + 0.5f, ry, LX1 - 0.5f, ry + ROWH, IM_COL32(226, 204, 160, 255));
        if (P.hit(LX0, ry, LX1, ry + ROWH)) V.pin_formula = V.pin_formula == k ? -1 : k;
        P.icon(LX0 + 4.5f, ry + ROWH * 0.5f, 5.8f, 95 + k / 3, -1, k % 3 + 1);
        P.text(f, 5.4f, LX0 + 9, ry + 0.9f, ink, gamedata.formula_names[k], -1, cx0 - 1);
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
    /* the formula */
    int k = hover >= 0 ? hover : V.pin_formula >= 0 ? V.pin_formula : n ? rows[sc] : -1;
    const float DX0 = 203, DX1 = 318, DW = DX1 - DX0 - 5;
    P.rect(DX0, 22, DX1, 197, IM_COL32(236, 224, 196, 255), 1.5f);
    if (k < 0 || k >= (int)gamedata.formulas.size()) return;
    const Formula &fm = gamedata.formulas[k];
    float y = 24;
    P.icon(DX0 + 6, y + 4, 8, 95 + k / 3, -1, k % 3 + 1);
    y += P.para(f, NsDraw::FS_HEAD, DX0 + 12, y, DW - 9, ink, gamedata.formula_names[k]) + 1.5f;
    static const char *RISK[3] = { "low risk", "some risk", "high risk" };
    char mr[64]; snprintf(mr, sizeof mr, "Mystic number %d, %s.", fm.mystic, fm.risk >= 0 && fm.risk < 3 ? RISK[fm.risk] : "risk unknown");
    y += P.para(fo, NsDraw::FS_SMALL, DX0 + 2.5f, y, DW, soft, mr) + 1;
    y += P.para(fo, NsDraw::FS_SMALL, DX0 + 2.5f, y, DW, IM_COL32(26, 64, 132, 255), game_text(fm.text.c_str(), fm.text.size())) + 2;
    P.text(fo, 4.3f, DX0 + 2.5f, y, soft, "Ingredients: needed / the party has"); y += 6;
    bool all = true;
    for (const FormulaIngr &g : fm.ingr) {
        int have = g.item >= 0 && g.item < 200 ? s.have[g.item] : 0;
        if (have < g.count) all = false;
        P.icon(DX0 + 5, y + 3, 5.5f, g.item);
        P.text(fo, NsDraw::FS_SMALL, DX0 + 9.5f, y + 0.6f, ink, ns_item_name(g.item), -1, DX1 - 20);
        char v[16]; snprintf(v, sizeof v, "%d / %d", g.count, have);
        P.text(fo, NsDraw::FS_SMALL, 0, y + 0.6f, have >= g.count ? NsDraw::GREEN : NsDraw::RED, v, DX1 - 3);
        y += 6.6f;
    }
    y += 1.5f;
    P.text(fo, 4.3f, DX0 + 2.5f, y, soft, "The chance of one batch"); y += 6;
    for (int m : al) {
        if (!s.formula[m][k]) continue;
        int pc = s.intel[m] + (s.alch[m] - fm.mystic) + 100;  /* (the alchemy screen's, without the Philosopher's Stone) */
        pc = pc < 0 ? 0 : pc > 99 ? 99 : pc;
        char v[64]; snprintf(v, sizeof v, "%s: %d%%", ns_gtext(s.names[m]).c_str(), pc);
        P.text(fo, NsDraw::FS_SMALL, DX0 + 4, y, ink, v); y += 6;
    }
    if (!all) P.para(fo, NsDraw::FS_SMALL, DX0 + 2.5f, y + 1, DW, dim, "The party lacks an ingredient: brewing must wait.");
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
void ns_saint_parts(const std::string &t, std::string &lore, std::string &eff) { saint_parts(t, lore, eff); }
