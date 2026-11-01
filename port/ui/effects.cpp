// effects.cpp - what is at work on the party now (effects.h): the game's events of a saint's boost or the
// lowering some saints bring with it, a weapon potion and a companion's stay, read from the table in memory
// (or, in a battle, from the file the cards left), each with its picture and how long it lasts. The margin's
// "Blessings and potions" (ui/margins.cpp margin_effects) and the character sheet (ui/sheetview.cpp) show it.
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <string>
#include <vector>
#include <array>
#include "imgui.h"
#include "gamedata.h"
#include "effects.h"
#include "../mods/fixes.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/hle.h"
#include "../mods/gamemodel.h"
}

/* hours since year 0 (the game's calendar: 365 days, no leap years; month from 0, day from 1) */
static long abs_hours(int y, int mo, int d, int h)
{
    static const int DAYS[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    long days = (long)y * 365;
    for (int i = 0; i < mo && i < 12; i++) days += DAYS[i];
    return (days + d) * 24 + h;
}

/* the running temporary effects: events of kind 0x48 (attribute: a saint's boost, or the
   lowering some saints bring with it), 0x49 (skill), 0x4a (weapon potion); +0 what, +0x1a member,
   +0x1e amount, +0x12 end (hour, day, month, year) */
typedef std::array<unsigned char, 0x30> EvRec;
static std::vector<EvRec> table_records(void)            /* the event table in memory */
{
    std::vector<EvRec> r;
    for (int i = 0; i < G_NEVENTS; i++) {
        u32 p = g_event(i);
        if (!p) continue;
        EvRec e; for (int k = 0; k < 0x30; k++) e[k] = rb((u16)(p >> 16), (u16)((u16)p + k));
        r.push_back(e);
    }
    return r;
}
/* the table as the cards left it when a battle closed them (cards_shutdown, s043 1462:054e: events.tmp in the
   game's folder, a count and the records; cards_startup reads it back after the battle) */
static std::vector<EvRec> file_records(void)
{
    std::vector<EvRec> r;
    char p[1200]; dos_host_path("EVENTS.TMP", p, sizeof p);
    FILE *f = fopen(p, "rb");
    if (!f) return r;
    unsigned char c[2];
    if (fread(c, 1, 2, f) == 2) {
        int n = c[0] | c[1] << 8;
        for (int i = 0; i < n && i < G_NEVENTS; i++) { EvRec e; if (fread(e.data(), 1, 0x30, f) != 0x30) break; r.push_back(e); }
    }
    fclose(f);
    return r;
}
static std::vector<Eff> effects_table(int size, const std::vector<EvRec> &recs)
{
    std::vector<Eff> v;
    long now = abs_hours((s16)g_rw(G_CUR_YEAR), (s16)g_rw(G_CUR_MONTH), (s16)g_rw(G_CUR_DAY), (s16)g_rw(G_CUR_HOUR));
    for (const EvRec &R : recs) {
        auto W = [&](int off) { return (int)(s16)(R[off] | R[off + 1] << 8); };
        int kind = W(0x22);
        if (kind == 0x43) {                                 /* a companion's stay (add_member: type0 the slot; its end, the day they leave) */
            int m = W(0);
            long left = abs_hours(W(0x18), W(0x16), W(0x14), W(0x12)) - now;
            if (m >= 0 && m < size && left > 0 && left < 24L * 365 * 50) { Eff e = { m, "stays with the party", left, kind }; e.icon = EI_STAYS; v.push_back(e); }
            continue;
        }
        if (kind < 0x48 || kind > 0x4a) continue;
        int m = W(0x1a), k = W(0), t = W(0x1e);
        if (m < 0 || m >= size || t == 0 || (t < 0 && kind == 0x4a)) continue;     /* (a saint's lowering of an attribute: negative) */
        long left = abs_hours(W(0x18), W(0x16), W(0x14), W(0x12)) - now;
        if (left <= 0) continue;
        char b[96];
        if ((kind == 0x48 && k >= 0 && k < 7) || (kind == 0x49 && k >= 0 && k < 19)) {
            snprintf(b, sizeof b, "%+d %s", t, kind == 0x48 ? gm_attr_name(k) : gm_skill_name(k));
            Eff e = { m, b, left, kind }; e.k = k; e.amount = t;
            v.push_back(e);
            continue;
        }
        if (kind == 0x4a) {
            /* +0: 0x69 Deadly Blade; 0x6d Hardarmor (its grade), 0x16d a saint's armour, 0x16e a saint's
               armour quality, 0x174 a saint's fire protection (armourday); 0x74 Firewall (armourday);
               0x6b Greatpower marked by greatpower; else an inventory index:
               Strongedge (3..5) or Greatpower (14/22/30) */
            /* (a saint's, mods/sainttexts.c: 0x16f-0x173 armour and weapon quality, 0x175 armour against
               missiles, 0x177 / 0x178 the party's travel faster) */
            static const struct { int k; const char *f; int icon; const char *v; } SAINTLY[] = {
                { 0x16d, "%+d armor (from a saint)", EI_ARMOR, "%+d" }, { 0x16e, "worn armor %+d quality (from a saint)", EI_ARMOR_Q, "%+dq" },
                { 0x16f, "metal armor %+d quality (from a saint)", EI_ARMOR_Q, "%+dq" }, { 0x170, "leather and cloth armor %+d quality (from a saint)", EI_ARMOR_Q, "%+dq" },
                { 0x171, "weapon %+d quality (from a saint)", EI_WEAPON_Q, "%+dq" }, { 0x172, "edged weapon %+d quality (from a saint)", EI_WEAPON_Q, "%+dq" },
                { 0x173, "blades, maces, polearms, crossbows and guns %+d quality (from a saint)", EI_WEAPON_Q, "%+dq" },
                { 0x174, "fire protection %d (from a saint)", EI_FIRE, "%d" }, { 0x175, "%+d armor against missiles (from a saint)", EI_MISSILE_ARMOR, "%+d" },
                { 0x177, "the party travels %d%% faster (from a saint)", EI_TRAVEL, "+%d%%" }, { 0x178, "the party travels %d%% faster in woods (from a saint)", EI_TRAVEL_WOODS, "+%d%%" },
                { 0x179, "no death from wounds (from a saint)", EI_NO_DEATH, "" } };
            bool done = false;
            for (const auto &x : SAINTLY) if (k == x.k) {
                char vb[16]; snprintf(vb, sizeof vb, x.v, t);
                if ((k == 0x177 || k == 0x178) && (t == 50 || t == 100)) {   /* (the speed in words) */
                    snprintf(b, sizeof b, "the party travels %s%s (from a saint)", t == 100 ? "twice as fast" : "half again as fast", k == 0x178 ? " in woods" : "");
                    snprintf(vb, sizeof vb, "%s", t == 100 ? "x2" : "x1.5");
                } else snprintf(b, sizeof b, x.f, t);
                Eff e = { m, b, left, kind }; e.k = k; e.amount = t; e.icon = x.icon; e.src = 2; e.val = vb; v.push_back(e); done = true; break; }
            if (done) continue;
            int item = k == 0x69 || k == 0x6b || k == 0x6d || k == 0x74 ? k : t <= 5 ? 0x6a : 0x6b;
            std::string n = item < (int)gamedata.items.size() ? game_text(gamedata.items[item].name.c_str(), 64) : "Potion";
            char vb[24] = ""; int icon = EI_DAMAGE;
            if (item == 0x69) { snprintf(b, sizeof b, "%s: %+d damage", n.c_str(), t); snprintf(vb, sizeof vb, "%+d", t); }
            else if (item == 0x6a) { snprintf(b, sizeof b, "%s: %+d penetration", n.c_str(), t); snprintf(vb, sizeof vb, "%+d", t); icon = EI_PENETRATION; }
            else if (item == 0x6d) { static const int T[4] = { 0, 1, 2, 2 }, Q[4] = { 0, 20, 25, 30 }; int g = t < 1 ? 1 : t > 3 ? 3 : t;
                snprintf(b, sizeof b, "%s: armor %+d thickness, %+d quality", n.c_str(), T[g], Q[g]); snprintf(vb, sizeof vb, "%+d, %+dq", T[g], Q[g]); icon = EI_ARMOR; }
            else if (item == 0x74) { snprintf(b, sizeof b, "%s: fire protection %d", n.c_str(), t); snprintf(vb, sizeof vb, "%d", t); icon = EI_FIRE; }
            else if (fix_is_on("greatpower")) { snprintf(b, sizeof b, "%s: weapon quality %+d", n.c_str(), t); snprintf(vb, sizeof vb, "%+dq", t); icon = EI_WEAPON_Q; }
            else if (!(t >> 4)) { snprintf(b, sizeof b, "%s: %+d penetration", n.c_str(), t & 15); snprintf(vb, sizeof vb, "%+d", t & 15); icon = EI_PENETRATION; }
            else { snprintf(b, sizeof b, "%s: %+d damage, %+d penetration", n.c_str(), t >> 4, t & 15); snprintf(vb, sizeof vb, "%+d, %+d", t >> 4, t & 15); }
            Eff e = { m, b, left, kind }; e.k = k; e.amount = t; e.icon = icon; e.src = 1; e.val = vb;
            v.push_back(e);
            continue;
        } else continue;
    }
    /* of the protections that do not add up (fire, the party's speed), only the strongest of each member's */
    std::vector<Eff> out;
    for (size_t i = 0; i < v.size(); i++) {
        const Eff &e = v[i];
        bool weaker = false;
        if (e.kind == 0x4a && (e.k == 0x174 || e.k == 0x177 || e.k == 0x178))
            for (size_t j = 0; j < v.size() && !weaker; j++) {
                const Eff &o = v[j];
                if (j != i && o.m == e.m && o.kind == 0x4a && o.k == e.k &&
                    (o.amount > e.amount || (o.amount == e.amount && (o.left > e.left || (o.left == e.left && j < i))))) weaker = true;
            }
        if (!weaker) out.push_back(e);
    }
    return out;
}
/* what is at work now. While a battle has closed the cards the event table holds only what the
   battle made (mods/fixes.c): what was at work when it began is kept from then and shown with it
   (its hours as they were; a battle takes minutes). A game loaded in a battle: what was at work is
   read from the table the cards left (events.tmp), once */
std::vector<Eff> effects_now(int size)
{
    static std::vector<Eff> before; static int before_seed = -1;
    int seed = (u16)g_rw(G_PARTY_SEED);
    std::vector<Eff> v = effects_table(size, table_records());
    if (!fixes_cards_down()) { before = v; before_seed = seed; return v; }
    {   /* (the file as the cards left it, read again only when it changed: a save loaded in the battle) */
        static long stamp = -2; static double checked = -10;
        double t = ImGui::GetTime();
        if (t - checked > 1.0 || seed != before_seed) {
            checked = t;
            char p[1200]; dos_host_path("EVENTS.TMP", p, sizeof p);
            struct stat st; long now = stat(p, &st) == 0 ? (long)st.st_mtime ^ ((long)st.st_size << 20) : -1;
            if (now != -1 && (now != stamp || seed != before_seed)) { before = effects_table(5, file_records()); stamp = now; before_seed = seed; }
        }
    }
    if (seed != before_seed) return v;
    for (const Eff &e : before) if (e.m < size) v.push_back(e);
    return v;
}

/* the roundels' names: in a text ({*Fire}: ui/richtext.h) and under the pointer, in the order of EI_* */
static const char *const EFFECT_NAME[] = { "Fire", "Armor", "Armor Quality", "Missile Armor", "Weapon Quality", "Damage", "Penetration", "Travel", "Woods Travel", "No Death", "Stays" };
static const char *const EFFECT_SAY[] = { "Fire protection", "Armor", "Armor quality", "Armor against missiles", "Weapon quality", "Damage", "Penetration",
                                          "Faster travel", "Faster travel in woods", "No death from wounds", "A companion's stay" };
static_assert(sizeof EFFECT_NAME / sizeof *EFFECT_NAME == EI_STAYS + 1 && sizeof EFFECT_SAY / sizeof *EFFECT_SAY == EI_STAYS + 1, "a name for each EI_*");
const char *ui_effect_name(int ei) { return ei >= 0 && ei <= EI_STAYS ? EFFECT_NAME[ei] : ""; }
const char *ui_effect_say(int ei) { return ei >= 0 && ei <= EI_STAYS ? EFFECT_SAY[ei] : ""; }
int ui_effect_of(const char *name, int len)
{
    for (int k = 0; k <= EI_STAYS; k++) if ((int)strlen(EFFECT_NAME[k]) == len && !strncmp(EFFECT_NAME[k], name, (size_t)len)) return k;
    return -1;
}
