/* teachers.c - the teachers a town can offer, and the card options that bring them (teachers.h). */
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "gamemodel.h"
#include "teachers.h"

enum { SK_EDGED = 0, SK_IMPACT = 1, SK_THROWN = 4, SK_BOWS = 5, SK_MISSILE = 6, SK_ALCHEMY = 7, SK_RELIGION = 8,
       SK_LATIN = 11, SK_RW = 12, SK_HEALING = 13, SK_ARTIFICE = 14 };

int teach_chance(int s)
{
    int t = (s * 20 / 100) * 33 / 10;                    /* practice_skill: rand(100) <= t */
    t += 1;
    return t < 0 ? 0 : t > 100 ? 100 : t;
}

static void add(TchSource *o, int skill, int fee) { if (o->n < TCH_MAXSK) { o->skill[o->n] = skill; o->fee[o->n] = fee; o->n++; } }

/* the guilds: quality index in shopq, the card's standing test ($MilCr00: seed offset, threshold; the
   artificers' street has none), the source */
static const struct { int src, q, off, need; const char *who, *ask; } GUILDS[5] = {
    { TCH_BLACKSMITHS, 0, 20, 100, "The blacksmiths' masters", "At the blacksmiths' guild, by day: \"request a meeting with the guild's masters\"." },
    { TCH_SWORDSMITHS, 2, 19, 90, "The swordsmiths' masters", "At the swordsmiths' guild, by day: \"request a meeting with the guild's masters\"." },
    { TCH_ARMORERS, 3, 18, 95, "The armorers' masters", "At the armorers' guild, by day: \"request a meeting with the guild's masters\"." },
    { TCH_BOWYERS, 5, 17, 85, "The bowyers' masters", "At the bowyers' and gunsmiths' guild, by day: \"request a meeting with the guild's masters\"." },
    { TCH_ARTIFICERS, 6, 0, 0, "The artificers' masters", "At the artificers' guild, by day: \"request a meeting with the guild's masters\"." },
};
static const char GUILD_COND[] = "The guild's hall receives a party of some standing: fame, the town's regard and the party's Charisma "
                                 "(else the street leads to the shop). While a dispute divides the guild, its masters want help with that first.";

static int is_here(int loc) { return loc >= 0 && loc == (s16)g_rw(GV_cur_loc); }

/* the guild's standing test for the party now (the town it is in): score - threshold */
static int standing(int k, int seed, int loc)
{
    int fame = (s16)g_rw(GV_party_fame), rep = 0, sum = 0, n = 0;
    u32 lr = g_location(loc);
    if (lr) rep = (s16)rw((u16)(lr >> 16), (u16)((u16)lr + 0x12));
    for (int m = 0; m < 5; m++) if (g_rb((u16)(G_MEMBER(m) + G_M_STATUS))) { sum += (s8)g_rb((u16)(G_MEMBER(m) + G_M_ATTR_CUR + 5)); n++; }
    int score = (int)((u16)(seed + GUILDS[k].off) % 21) + fame / 20 + rep / 2 + sum / (n ? n : 1);
    return score - GUILDS[k].need;
}

static int party_best_skill(int k)
{
    int b = 0;
    for (int m = 0; m < 5; m++) if (g_rb((u16)(G_MEMBER(m) + G_M_STATUS))) { int v = (s8)g_rb((u16)(G_MEMBER(m) + G_M_SKILLS + k)); if (v > b) b = v; }
    return b;
}

/* a started event of kind k at loc (and type0 t, or any with t < 0): its index, -1 none */
static int started(u16 s, u16 o)
{
    int y = (s16)rw(s, (u16)(o + 0x10)), mo = (s16)rw(s, (u16)(o + 0x0e)), d = (s16)rw(s, (u16)(o + 0x0c)), h = (s16)rw(s, (u16)(o + 0x0a));
    int cy = (s16)g_rw(G_CUR_YEAR), cm = (s16)g_rw(G_CUR_MONTH), cd = (s16)g_rw(G_CUR_DAY), ch = (s16)g_rw(G_CUR_HOUR);
    if (y != cy) return y < cy;
    if (mo != cm) return mo < cm;
    if (d != cd) return d < cd;
    return h <= ch;
}
static int event_at(int kind, int loc, int t)
{
    for (int i = 0; i < G_NEVENTS; i++) {
        u32 e = g_event(i); if (!e) continue;
        u16 s = (u16)(e >> 16), o = (u16)e;
        if ((s16)rw(s, (u16)(o + 0x22)) != kind || (s16)rw(s, (u16)(o + 0x1c)) != loc) continue;
        if (t >= 0 && (s16)rw(s, o) != t) continue;
        if (started(s, o)) return i;
    }
    return -1;
}
static int dispute_at(int loc)              /* ev8_at(2, loc): a guild's dispute here (the guild cards' option 2 leads to it) */
{
    for (int i = 0; i < G_NEVENTS; i++) {
        u32 e = g_event(i); if (!e) continue;
        u16 s = (u16)(e >> 16), o = (u16)e;
        if ((s16)rw(s, (u16)(o + 0x22)) == 8 && (s16)rw(s, (u16)(o + 0x28)) == 2 && (s16)rw(s, (u16)(o + 0x1c)) == loc && started(s, o)) return 1;
    }
    return 0;
}

int teach_hired(int loc, TchHired *out, int max)
{
    int n = 0;
    for (int i = 0; i < G_NEVENTS && n < max; i++) {
        u32 e = g_event(i); if (!e) continue;
        u16 s = (u16)(e >> 16), o = (u16)e;
        if ((s16)rw(s, (u16)(o + 0x22)) != 0x28 || (s16)rw(s, (u16)(o + 0x1c)) != loc || !started(s, o)) continue;
        TchHired *h = &out[n++];
        h->skill = (s16)rw(s, o); h->fee = (s16)rw(s, (u16)(o + 0x1e)); h->strength = (s16)rw(s, (u16)(o + 0x20));
        h->day = (s16)rw(s, (u16)(o + 0x14)); h->month = (s16)rw(s, (u16)(o + 0x16)); h->year = (s16)rw(s, (u16)(o + 0x18));
    }
    return n;
}

int teach_town(int loc, const TchTown *t, TchSource *out, int max)
{
    int n = 0, here = is_here(loc);
    u16 seed = (u16)(t->seed + (s16)g_rw(G_PARTY_SEED));
    #define NEXT() (n < max ? memset(&out[n], 0, sizeof out[n]), &out[n++] : (TchSource *)0)
    for (int k = 0; k < 5; k++) {
        int q = t->shopq[GUILDS[k].q];
        if (q <= 0) continue;
        TchSource *o = NEXT(); if (!o) return n;
        o->src = GUILDS[k].src; o->who = GUILDS[k].who; o->ask = GUILDS[k].ask;
        o->cond = GUILDS[k].need ? GUILD_COND : "While a dispute divides the guild, its masters want help with that first.";
        o->strength = q + (int)(seed % 15) - 5;
        switch (GUILDS[k].src) {
        case TCH_BLACKSMITHS: add(o, SK_EDGED, 50); add(o, SK_IMPACT, 50); break;
        case TCH_SWORDSMITHS: add(o, SK_EDGED, 50); break;
        case TCH_ARMORERS: add(o, SK_EDGED, 50); add(o, SK_ARTIFICE, 50); break;
        case TCH_BOWYERS: { int w = (int)((u16)((s16)g_rw(G_PARTY_SEED) + loc) % 3); add(o, w == 0 ? SK_BOWS : w == 1 ? SK_THROWN : SK_MISSILE, 50); } break;
        default: add(o, SK_ARTIFICE, 50); break;
        }
        if (here) {
            if (dispute_at(loc)) snprintf(o->now, sizeof o->now, "A dispute divides one of the guilds here: its masters want help with that first.");
            else if (GUILDS[k].need) {
                int d = standing(k, seed, loc);
                snprintf(o->now, sizeof o->now, d >= 0 ? "The guild's hall would receive the party now, by day." : "The guild's hall would not receive the party yet (the street leads to the shop).");
                o->off = d < 0;
            }
        }
    }
    if (t->buildings & 0x100) {
        TchSource *o = NEXT(); if (!o) return n;
        o->src = TCH_UNIVERSITY; o->who = "The university's masters";
        o->ask = "At the university, by day: \"a teacher you can hire\".";
        o->cond = "If they agree (Perception, Intellect, Latin, the party's best Alchemy and fame); a refusal shuts the door for a week and costs a little of the town's regard.";
        o->strength = (int)(seed % 15) + 45;
        add(o, SK_ALCHEMY, 90); add(o, SK_RELIGION, 70); add(o, SK_LATIN, 50); add(o, SK_RW, 50); add(o, SK_HEALING, 110);
    }
    if (t->buildings & 0x80) {
        TchSource *o = NEXT(); if (!o) return n;
        o->src = TCH_MONASTERY; o->who = "The monks";
        o->ask = "At the monastery: \"inquire about tutoring in various disciplines\".";
        o->cond = "For a party of good repute and some virtue (Virtue, Charisma, the town's regard), while no other teacher is hired in the town; a refusal holds for two days.";
        o->strength = 50;
        add(o, SK_RELIGION, 50); add(o, SK_LATIN, 25); add(o, SK_RW, 25);
    }
    if (t->physician) {
        TchSource *o = NEXT(); if (!o) return n;
        o->src = TCH_PHYSICIAN; o->who = "The physician";
        o->ask = "At the physician's, by day: \"beg him to allow you to be his students\".";
        o->cond = "Not to a party that already heals as well as he does; and some years he takes no students.";
        o->strength = 50;
        add(o, SK_HEALING, 60);
        int year = (s16)g_rw(G_CUR_YEAR);
        if ((s16)(seed + year) % 3 == 0) { snprintf(o->now, sizeof o->now, "He takes no students this year."); o->off = 1; }
        else if (here) {
            int e = event_at(0x37, loc, -1);
            if (e >= 0) {
                u32 p = g_event(e); int P = (s16)rw((u16)(p >> 16), (u16)((u16)p + 0x1e));
                if (P <= 1) { snprintf(o->now, sizeof o->now, "He takes no students now."); o->off = 1; }
                else if (party_best_skill(SK_HEALING) > P) { snprintf(o->now, sizeof o->now, "The party already heals better than he does: he has nothing to teach it."); o->off = 1; }
            }
        }
    }
    if (t->alchemist) {
        TchSource *o = NEXT(); if (!o) return n;
        o->src = TCH_ALCHEMIST; o->who = "The alchemists";
        o->ask = "In Astrologists' Lane, by day: \"receiving additional instruction in alchemy\".";
        o->cond = "If they agree (Charisma, Speak Common, the party's best Alchemy, the town's regard); a refusal holds for two and a half days.";
        o->strength = t->size * 3 + (int)((u16)(seed + 4) % 41) + ((t->buildings & 0x100) ? (int)((u16)(seed + 9) % 11) + 10 : 0);
        add(o, SK_ALCHEMY, o->strength / 4 + 12);
    }
    #undef NEXT
    /* what is hired here already: a guild, the physician make no second teacher of a skill; the monks
       none while any teacher is hired in the town */
    TchHired h[16]; int nh = teach_hired(loc, h, 16);
    for (int k = 0; k < n; k++) {
        TchSource *o = &out[k];
        if (o->off) continue;
        if (o->src == TCH_MONASTERY && nh) { snprintf(o->now, sizeof o->now, "Not while teachers are hired in the town."); o->off = 1; continue; }
        if (o->src == TCH_ARTIFICERS || o->src == TCH_UNIVERSITY || o->src == TCH_ALCHEMIST) continue;
        int all = o->n > 0;
        for (int i = 0; i < o->n; i++) { int got = 0; for (int j = 0; j < nh; j++) if (h[j].skill == o->skill[i]) got = 1; if (!got) all = 0; }
        if (all) { snprintf(o->now, sizeof o->now, "Hired already: at the inn now."); o->off = 1; }
    }
    return n;
}

void teach_town_here(TchTown *t)
{
    memset(t, 0, sizeof *t);
    u16 off = g_rw(GV_city_rec), sg = g_rw((u16)(GV_city_rec + 2));
    if (!sg) return;
    t->size = (s16)rw(sg, (u16)(off + 0x40));
    t->buildings = rw(sg, (u16)(off + 0x5e));
    t->seed = (s16)rw(sg, (u16)(off + 0x56));
    for (int k = 0; k < 9; k++) t->shopq[k] = (s8)rb(sg, (u16)(off + 0x62 + k));
    t->physician = t->alchemist = 1;                     /* (on their own cards: there) */
}

/* the card's option and the source it brings */
static const struct { const char *card; int opt, src; } OPTS[] = {
    { "$Black00", 2, TCH_BLACKSMITHS }, { "$Sword00", 2, TCH_SWORDSMITHS }, { "$Armor00", 2, TCH_ARMORERS },
    { "$Bowye00", 2, TCH_BOWYERS }, { "$Artif00", 2, TCH_ARTIFICERS }, { "$Guild00", 0, TCH_GUILD },
    { "$Unive00", 4, TCH_UNIVERSITY }, { "$CityM00", 1, TCH_MONASTERY }, { "$Physi00", 2, TCH_PHYSICIAN },
    { "$Alche00", 4, TCH_ALCHEMIST },
};

int teach_card_option(const char *card, int opt, TchSource *out)
{
    int src = -1;
    for (size_t k = 0; k < sizeof OPTS / sizeof OPTS[0]; k++)
        if (OPTS[k].opt == opt && !strncasecmp(card, OPTS[k].card, 8)) { src = OPTS[k].src; break; }
    if (src < 0) return 0;
    int loc = (s16)g_rw(GV_cur_loc);
    TchTown t; teach_town_here(&t);
    if (src == TCH_GUILD) {                              /* $Guild00 (sub_s070_19af_0394): by the guild (D_e3d0) */
        int bit = (s16)g_rw(0xe3d0);
        u16 seed = (u16)(t.seed + (s16)g_rw(G_PARTY_SEED));
        memset(out, 0, sizeof *out);
        out->src = TCH_GUILD; out->who = "The guild's masters";
        out->ask = "Here: \"inquire about special training\".";
        out->cond = "If they agree (Charisma, the town's regard and fame; sure when the guild owes the party).";
        int q = bit == 2 || bit == 0x10 ? t.shopq[0] : bit == 4 ? t.shopq[2] : bit == 8 ? t.shopq[5] : t.shopq[6];
        out->strength = q + (int)(seed % 15) - 5;
        if (bit == 8) snprintf(out->now, sizeof out->now, "The game's own slip: this guild's teacher teaches no skill at all.");
        else if (bit == 2 || bit == 0x10 || bit == 4) { add(out, SK_EDGED, 50); add(out, SK_IMPACT, 50); }
        else add(out, SK_ARTIFICE, 50);
        return 1;
    }
    TchSource all[12];
    int n = teach_town(loc, &t, all, 12);
    for (int k = 0; k < n; k++) if (all[k].src == src) { *out = all[k]; return 1; }
    return 0;
}
