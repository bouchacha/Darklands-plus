/* politics.c - the political campaign MicroProse wrote and never switched on (docs/cut_politics.md),
 * restored as an experimental option (rules.politics, off in every preset).
 *
 * The game has a city's "political season" (s039 1462:0558: 15 interest groups each leaning to
 * the rebels or the government, the guild where the agitators meet, 56 days), 19 $Revol cards and
 * their helpers, all coded, but nothing calls the season's setup, the war council ($Consi00) keeps
 * only "back to work", and the decisive day (facts 0x15 "the guard arrests today" and 0x16 "the
 * rebels march today", read by every street card) is written by nothing. This module:
 *  - starts a season now and then when the party arrives in a city (one at a time), putting right
 *    what the setup gets wrong: the party's side starts at the city's number instead of 0 (the
 *    season's own record doubles as fact 0, so the agitators' guild master never offered the job),
 *    the agitators' guild can be the merchants (no guild card), no guild leader is given a secret;
 *  - makes the war council work from its texts (the tally, the plan, "put the plan into effect,
 *    three days from now") and writes the decisive day; the other side strikes on its own when it
 *    is stronger or the season grows old; a day the party missed is settled when it comes back;
 *  - lets the strategy talk offer the march or the arrest by the city's layout (the game asked
 *    for an imperial mint, a slip for the town hall), and on the day sends the party at
 *    headquarters to the rally or the arrest ($Revol09, $Revol08: nothing led there);
 *  - opens the street gossip ($Revol04, from the rumours of $Speci00) and writes its two empty
 *    options (government and rebel activities) from its texts;
 *  - fills the texts' $VarString tokens (the engine has none: they showed as written) and turns
 *    $Support the right way round (the groups' -1 is the rebels' side in every rule, "oppose" in
 *    the names table);
 *  - puts right $Revol13's bribe (its flag was overwritten with the saint's: no money needed) and
 *    the news afterwards (conflict_outcome marks every city "overthrown", whoever won).
 * Everything the season knows lives in the game's own events, so it is saved with the game. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"
#include "gamemodel.h"

void ui_notice(const char *);

#define TAG       2              /* the season's facts */
#define F_SIDE    0              /* the party's side: -1 rebels, 1 government (the season record itself) */
#define F_SECRET  8              /* guild bits: its leader has a secret */
#define F_AGITATE 0x17           /* the agitators' guild (one bit) */
#define F_ARREST  0x15           /* the guard arrests today */
#define F_MARCH   0x16           /* the rebels march today */
#define F_DAYOVER 0x27           /* (ours) starts when the decisive day is over; value: who acted */
#define REBELS   (-1)
#define GOVT       1
#define H_MARCH    9             /* Terce */
#define H_ARREST   4             /* Lauds */
#define DAY_HOURS 12

static int dlog(void) { static int v = -1; if (v < 0) v = getenv("DL_POLITICS_LOG") != 0; return v; }
#define LOG(...) do { if (dlog()) fprintf(stderr, "politics: " __VA_ARGS__); } while (0)

int politics_on(void) { return config_get_int("rules.politics", 0) && !config_get_int("original", 0); }

/* ---------------------------------------------------------------- the game */
#define C(...) ns_call(__VA_ARGS__)
static int cur_loc(void) { return (s16)g_rw(G_CUR_LOC); }
static int in_city(void) { int l = cur_loc(); return l >= 0 && l < 0x5c; }
static int season(int loc) { return C(GF_ev8_at, 2, TAG, loc); }
static int fact(int type) { return C(GF_loc_fact, 3, TAG, type, cur_loc()); }
static int fact_value(int type) { return C(GF_loc_fact_value, 3, TAG, type, cur_loc()); }
static int side(void) { int v = fact_value(F_SIDE); return v == REBELS || v == GOVT ? v : 0; }
static int side_get(int bit) { return C(GF_loc_side_get_impl, 1, bit); }
static void side_set(int bit, int v) { C(GF_loc_side_set_impl, 2, bit, v); }
static int rebels_pct(void) { return C(GF_side1_percent_impl, 0); }     /* the rebels' strength, in % of the government's */
static int govt_pct(void) { return C(GF_side2_percent_impl, 0); }
static int rnd(int n) { return C(GF_rand_mod, 1, n); }
static void card_text(int b) { C(GF_card_text, 2, b, 0); }
static u16 city_word(int off) { u16 o = g_rw(GV_city_rec), s = g_rw((u16)(GV_city_rec + 2)); return s ? rw(s, (u16)(o + off)) : 0; }
static u8 city_byte(int off) { u16 o = g_rw(GV_city_rec), s = g_rw((u16)(GV_city_rec + 2)); return s ? rb(s, (u16)(o + off)) : 0; }
static int has_council(void) { return city_word(0x5e) & 2; }
static int has_fortress(void) { return city_word(0x5e) & 4; }
static int lord_city(void) { return C(GF_cur_loc_query, 1, 0x21) % 2 == 0; }   /* ($Revol02: the rebels serve the exiled lord, not a new council) */
static int loc_flags(int loc) { u32 r = g_location(loc); return r ? rb((u16)(r >> 16), (u16)((u16)r + 0x14)) : 0; }
static void loc_flags_set(int loc, int v) { u32 r = g_location(loc); if (r) wb((u16)(r >> 16), (u16)((u16)r + 0x14), (u8)v); }
static void dset(u16 off, const char *s, int max) { int i = 0; for (; s[i] && i < max - 1; i++) g_wb((u16)(off + i), (u8)s[i]); g_wb((u16)(off + i), 0); }
/* a $variable's value, expanded by the game on its own stack */
static void expand(const char *var, char *out, int n)
{
    u16 sp0 = SP, buf = (u16)((SP - 80) & ~1);
    SP = buf;
    dset(buf, var, 80);
    C(GF_var_expand, 1, buf);
    g_str(G_DGROUP, buf, out, n);
    SP = sp0;
    if (ns_unwinding) out[0] = 0;
}
static int card_is(const char *n) { char nm[16]; g_str(G_DGROUP, GV_card_name, nm, sizeof nm); return !strcasecmp(nm, n); }

/* the guilds: the side bit the cards use, and the setup's index (loc_query(loc, d + 3): its shop) */
static const struct { int bit, shop_off; } GUILD[7] = {
    { 2, 0x62 }, { 4, 0x64 }, { 8, 0x67 }, { 0x10, 0x65 }, { 0x20, 0x68 }, { 0x40, 0x6a }, { 0x80, 0x69 } };

/* ---------------------------------------------------------------- dates */
static int leap(int y) { return y % 4 == 0; }                          /* (the Julian calendar) */
static int mdays(int m, int y) { static const int D[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }; return D[m] + (m == 1 && leap(y)); }
static long serial(int y, int m, int d, int h)                        /* hours since a fixed day */
{
    long days = (long)y * 365 + y / 4;
    for (int k = 0; k < m; k++) days += mdays(k, y);
    return (days + d) * 24 + h;
}
static long now(void) { return serial((s16)g_rw(G_CUR_YEAR), (s16)g_rw(G_CUR_MONTH), (s16)g_rw(G_CUR_DAY), (s16)g_rw(G_CUR_HOUR)); }
static long ev_start(u32 e) { return serial(g_ev(e, 0x10), g_ev(e, 0x0e), g_ev(e, 0x0c), g_ev(e, 0x0a)); }
static const char *MONTHS[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
void gm_hour_name(int hour, char *out, int n);                         /* (mods/gamemodel.c: the game's own names of the bells) */
/* "d days from now at hour h": the hours until then, and the date as text */
static int hours_to(int d, int h, char *date, int n)
{
    int y = (s16)g_rw(G_CUR_YEAR), m = (s16)g_rw(G_CUR_MONTH), day = (s16)g_rw(G_CUR_DAY);
    for (int k = 0; k < d; k++) if (++day > mdays(m, y)) { day = 1; if (++m > 11) { m = 0; y++; } }
    if (date) snprintf(date, (size_t)n, "%d %s", day, MONTHS[m]);
    return d * 24 + h - (s16)g_rw(G_CUR_HOUR);
}

/* the season's record (kind 8, tag 2, at loc), and the events of the season */
static u32 season_rec(int loc)
{
    for (int i = 0; i < G_NEVENTS; i++) { u32 e = g_event(i); if (e && g_ev(e, G_EV_KIND) == 8 && g_ev(e, G_EV_TAG) == TAG && g_ev(e, G_EV_LOC) == loc) return e; }
    return 0;
}
static int any_season(void)
{
    for (int i = 0; i < G_NEVENTS; i++) { u32 e = g_event(i); if (e && g_ev(e, G_EV_KIND) == 8 && g_ev(e, G_EV_TAG) == TAG) return 1; }
    return 0;
}
static u32 fact_rec(int type, int loc)                                 /* started or not */
{
    for (int i = 0; i < G_NEVENTS; i++) {
        u32 e = g_event(i);
        if (e && g_ev(e, G_EV_KIND) == 0x1c && g_ev(e, G_EV_TAG) == TAG && g_ev(e, G_EV_TYPE0) == type && g_ev(e, G_EV_LOC) == loc) return e;
    }
    return 0;
}
static int season_days(int loc) { u32 e = season_rec(loc); return e ? (int)((now() - ev_start(e)) / 24) : 0; }
/* the decisive day set: by whom (REBELS, GOVT), 0 none */
static int day_set(int loc) { u32 e = fact_rec(F_DAYOVER, loc); return e ? g_ev(e, G_EV_VALUE) : 0; }
static int day_hours_left(int loc)                                    /* until the day starts (<= 0: it has) */
{
    u32 e = fact_rec(F_ARREST, loc); if (!e) e = fact_rec(F_MARCH, loc);
    return e ? (int)(ev_start(e) - now()) : -9999;
}

/* a fact of the season from h hours on, lasting len hours */
static void fact_from(int loc, int type, int value, int h, int len)
{
    int i = C(GF_event_new, 10, -2, loc, 0x5a, 0x1c, type, value, 0, h + len, 0, TAG);
    if (ns_unwinding || i < 0) return;
    u32 e = g_event(i);
    if (e && h > 0) C(GF_date_add_hours, 3, (u16)((u16)e + G_EV_START), (u16)(e >> 16), h);
}
/* the decisive day: who acts, d days from now */
static void set_day(int loc, int who, int d)
{
    int h = hours_to(d, who == REBELS ? H_MARCH : H_ARREST, 0, 0);
    fact_from(loc, who == REBELS ? F_MARCH : F_ARREST, who, h, DAY_HOURS);
    fact_from(loc, F_DAYOVER, who, h + DAY_HOURS, 24 * 40);
    LOG("the %s act in %d hours (day %d of the season)\n", who == REBELS ? "rebels" : "government", h, season_days(loc));
}

/* ---------------------------------------------------------------- the season begins */
static int eligible(int loc)
{
    if (loc_flags(loc) & 0x80 || loc_flags(loc) == 2) return 0;      /* (overthrown before; the setup refuses kind 2) */
    if (C(GF_event_at, 2, 0x42, loc)) return 0;                     /* (a rebellion put down lately) */
    for (int g = 0; g < 6; g++) if (city_byte(GUILD[g].shop_off)) return 1;   /* (a guild with a street: the setup loops forever without) */
    return 0;
}
int politics_start(int loc)
{
    if (loc != cur_loc() || !in_city() || season(loc) || !eligible(loc)) return 0;
    C(39, 0x1462, 0x0558, 1, loc);                                   /* the season's setup, never called by the game */
    if (ns_unwinding || !season(loc)) return 0;
    C(GF_event_update, 7, -2, loc, 0x1c, F_SIDE, 0, TAG, 0);         /* the party on no side (the setup left the city's number) */
    int ag = fact_value(F_AGITATE), have[6], n = 0;
    for (int g = 0; g < 6; g++) if (city_byte(GUILD[g].shop_off)) have[n++] = GUILD[g].bit;
    int known = 0; for (int k = 0; k < n; k++) known |= have[k] == ag;
    if (!known && n) { ag = have[rnd(n)]; C(GF_event_update, 7, -2, loc, 0x1c, F_AGITATE, ag, TAG, 0); }   /* (it was the merchants: no guild card) */
    side_set(ag, REBELS);                                            /* the agitators' guild is for change */
    int secrets = 0;
    for (int g = 0; g < 7; g++) if (city_byte(GUILD[g].shop_off) && GUILD[g].bit != ag && rnd(3) == 0) secrets |= GUILD[g].bit;
    C(GF_event_new, 10, -2, loc, 0x5a, 0x1c, F_SECRET, secrets, 0, 0x540, 0, TAG);
    LOG("a season begins at %d: agitators %#x, secrets %#x, rebels %d%%\n", loc, ag, secrets, rebels_pct());
    return 1;
}

/* ---------------------------------------------------------------- the street: the season runs */
static void settle_missed(int loc)
{
    int who = day_set(loc), pct = who == REBELS ? rebels_pct() : govt_pct();
    int chance = pct / 2; if (chance < 10) chance = 10; if (chance > 90) chance = 90;
    int won = rnd(100) < chance, winner = won ? who : -who, party = side();
    char seat[48]; expand(has_council() ? "councilHall" : "fortress", seat, sizeof seat);
    if (!seat[0]) snprintf(seat, sizeof seat, "%s", has_council() ? "town hall" : "fortress");
    LOG("the day passed without the party: %s acted, %s won\n", who == REBELS ? "rebels" : "government", winner == REBELS ? "rebels" : "government");
    C(GF_conflict_outcome_impl, 3, who, 0, winner);                 /* (the party, 0: no reward, no blame) */
    if (ns_unwinding) return;
    char msg[240];
    if (who == REBELS) snprintf(msg, sizeof msg, won ? "While you were away, the rebels marched on the %s: the city has a new government."
                                                     : "While you were away, the rebels marched on the %s and were scattered: the old government holds.", seat);
    else snprintf(msg, sizeof msg, won ? "While you were away, the city guard arrested the rebel leaders: the old government holds."
                                       : "While you were away, the guard tried to arrest the rebel leaders and the crowd freed them: the city has a new government.");
    if (party) ui_notice(msg);
}
static int arrived = -1;
static long last_day = -1;
static void street(void)
{
    int loc = cur_loc();
    if (loc != arrived) {
        arrived = loc;
        if (!any_season() && eligible(loc) && rnd(6) == 0) politics_start(loc);
        if (ns_unwinding) return;
    }
    if (!season(loc)) return;
    long today = now() / 24;
    if (today != last_day) {                                         /* a new day: the guilds can be asked, met and visited again */
        last_day = today;                                            /* (the cards mark each once for the whole season: the leader's
                                                                        "report here every day" went through the agitators' guild) */
        for (int t = 3; t <= 5; t++) C(GF_event_update, 7, -2, loc, 0x1c, t, 0, TAG, 0);
        if (ns_unwinding) return;
    }
    u32 over = fact_rec(F_DAYOVER, loc);
    if (over && ev_start(over) <= now()) { settle_missed(loc); return; }
    int s = side();
    if (!s || over) return;                                          /* (the party has no side, or the day is set) */
    int days = season_days(loc), opp = s == REBELS ? govt_pct() : rebels_pct();
    if ((days >= 21 && opp >= 100) || days >= 35) set_day(loc, -s, 2);   /* the other side strikes first */
}

/* ---------------------------------------------------------------- texts */
static int cur_block = -1;                                           /* the text block being expanded */
static const char *support_word(int v) { return v < 0 ? "support" : v > 0 ? "oppose" : "are undecided about"; }
static void var_expand_hook(void)                                    /* var_expand (char *name): the value in place */
{
    if (!politics_on()) { hook_call_original(); return; }
    u16 p = rw(SS, (u16)(SP + 4));
    char nm[24]; g_str(G_DGROUP, p, nm, sizeof nm);
    int k = -1;
    if (!strcmp(nm, "Support")) k = 0;
    else if (!strncmp(nm, "Support", 7) && nm[7] >= '1' && nm[7] <= '6' && !nm[8]) k = nm[7] - '0';
    if (k >= 0) { dset(p, support_word((s16)g_rw((u16)(GV_var_support0 + 2 * k))), 40); hook_return_far(0); return; }
    if (!strcmp(nm, "VarString1") && (card_is("$Jewel00") || card_is("$Barbe00") || card_is("$Soldi00"))) {
        dset(p, support_word((s16)g_rw(GV_var_support0)), 40); hook_return_far(0); return;
    }
    const char *v = 0; char buf[64];
    if (!strcmp(nm, "VarString1") && card_is("$Revol02") && cur_block == 8) v = "the exiled heir";
    else if (!strcmp(nm, "VarString2") && card_is("$Revol09")) v = "banner";
    else if (!strcmp(nm, "VarString2")) v = has_council() ? "town hall" : "fortress";
    else if (!strcmp(nm, "VarString1") || !strcmp(nm, "VarString5")) {
        dset(p, has_council() ? "councilHall" : "fortress", 24);
        hook_call_original();                                        /* the building's name in this city */
        char b[48]; g_str(G_DGROUP, p, b, sizeof b);
        snprintf(buf, sizeof buf, "the %s", b); dset(p, buf, 60);
        return;
    }
    if (v) { dset(p, v, 40); hook_return_far(0); return; }
    hook_call_original();
}
static void text_expand_hook(void)                                   /* text_expand (idx, out): "the the" -> "the" */
{
    u16 out = rw(SS, (u16)(SP + 6));
    int was = cur_block, blk = (s16)rw(SS, (u16)(SP + 4));
    cur_block = blk;
    hook_call_original();
    cur_block = was;
    if (!politics_on()) return;
    char t[2048]; int n = 0; u8 c;
    while (n < (int)sizeof t - 1 && (c = g_rb((u16)(out + n)))) t[n++] = (char)c;
    t[n] = 0;
    int changed = 0;
    for (char *q; (q = strstr(t, "the the ")) || (q = strstr(t, "The the ")); changed = 1) memmove(q + 4, q + 8, strlen(q + 8) + 1);
    char *q = strstr(t, "$CityLord Title");                         /* ($Revol11: a space in the token) */
    if (q) {
        char v[48]; expand("CityLordTitle", v, sizeof v);
        if (v[0] && strlen(t) + strlen(v) < sizeof t) {
            memmove(q + strlen(v), q + 15, strlen(q + 15) + 1); memcpy(q, v, strlen(v)); changed = 1;
        }
    }
    if (card_is("$Revol11")) {
        for (char *r; (r = strstr(t, ", ?")); changed = 1) memmove(r, r + 2, strlen(r + 2) + 1);   /* (its boon names, if unset) */
        if (blk == 2) for (char *r = t; *r; r++) if (*r == 0x10 || *r == 0x16) { *r = 0x15; changed = 1; }   /* (the boons: plain options, below) */
    }
    if (changed && (int)strlen(t) < n + 16) for (int i = 0; ; i++) { g_wb((u16)(out + i), (u8)t[i]); if (!t[i]) break; }
}

/* ---------------------------------------------------------------- the cards */
static void card_load_hook(void)                                     /* card_load (name, pic): after it, per card */
{
    hook_call_original();
    if (!politics_on() || ns_unwinding) return;
    if (!in_city()) { arrived = -1; return; }
    static const char *STREETS[] = { "$citys00", "$citys01", "$MainS01", "$MainS02", "$SideS00", "$SideS01", "$Churc00", "$Marke00" };
    for (unsigned k = 0; k < sizeof STREETS / sizeof *STREETS; k++) if (card_is(STREETS[k])) { street(); return; }
    if (!season(cur_loc())) return;
    int r = rebels_pct(), overall = r > 110 ? REBELS : r < 90 ? GOVT : 0;
    if (card_is("$Consi00")) {                                       /* the war council: the tally, then the plan */
        int pro = 0, contra = 0;
        static const int BITS[11] = { 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80, 0x200, 0x400, 0x1000 };   /* (the groups the strengths count) */
        for (int k = 0; k < 11; k++) { int v = side_get(BITS[k]); pro += v == REBELS; contra += v == GOVT; }
        g_ww(GV_var_number1, (u16)pro); g_ww(GV_var_number2, (u16)contra);
        g_ww(GV_var_support0, (u16)overall);
        int s = side(); char date[40];
        int target = s == GOVT ? H_ARREST : H_MARCH;
        hours_to(3, target, date, sizeof date);
        char b[24]; gm_hour_name(target, b, sizeof b);
        dset(GV_var_text1, b, 40); dset(GV_var_text2, date, 40);
        u16 p = (u16)GV_var_text3;                                   /* the seat, by its name */
        dset(p, has_council() ? "councilHall" : "fortress", 24); C(GF_var_expand, 1, p);
        dset(GV_var_text4, "the exiled heir", 40);
        g_ww(GV_var_chosen1, (u16)(s8)g_rb(G_LEADER));
        card_text(0);
    } else if (card_is("$Revol04")) {
        g_ww(GV_var_support0, (u16)overall);
        int seed = (s16)city_word(0x56);                             /* the two leaders' names, as the rumours give them ($Speci00) */
        C(GF_random_name, 3, seed + 0xc, C(GF_cur_loc_query, 1, 0x21) % 5 ? 0 : 1, 1);
        C(GF_random_name, 3, seed + 0x16, 2, 0);
    } else if (card_is("$Polit01")) {                                /* the bribe: a tenth of the purse (its $Money1 was never set) */
        long pf = (long)(s16)g_rw(G_MONEY) * 240 + (s16)g_rw(G_MONEY + 2) * 12 + (s16)g_rw(G_MONEY + 4);
        pf /= 10;
        C(GF_money_set, 4, GV_var_money1, (int)(pf / 240), (int)(pf % 240 / 12), (int)(pf % 12));
    } else if (card_is("$Revol11")) {                                /* (its boons' "$Text2", "$Text3": names it never sets) */
        g_wb(GV_var_text2, 0); g_wb(GV_var_text3, 0);
    }
}

static int plan_blocked(int loc) { return day_set(loc) != 0; }
static void card_options_hook(void)                                  /* card_options (block): opt_on[] put right */
{
    if (politics_on() && in_city() && season(cur_loc())) {
        int loc = cur_loc(), s = side();
#define ON(i, v) g_wb((u16)(GV_opt_on + (i)), (u8)(v))
#define OPT(i) g_rb((u16)(GV_opt_on + (i)))
        if (card_is("$Revol03")) {                                   /* the march by the city's layout (the game asked for a mint) */
            int free = !plan_blocked(loc);
            ON(5, free && s == REBELS && has_council());
            ON(6, free && s == REBELS && !has_council() && has_fortress());
            ON(7, free && s == GOVT);
        } else if (card_is("$Consi00")) {
            int free = !fact_rec(F_DAYOVER, loc) || day_set(loc) == -s;   /* (the other side's plan: the leader says why not now) */
            for (int i = 0; i < 10; i++) ON(i, 0);
            ON(0, s == REBELS); ON(1, s == GOVT);
            ON(2, s == REBELS && free); ON(3, s == GOVT && free);
            ON(6, 1);
        } else if (card_is("$Snuck00")) {                            /* (a blank line, its handler empty) */
            ON(3, 0);
        } else if (card_is("$Revol09")) {                            /* (lines "2" to "5": options never written) */
            for (int i = 2; i < 6; i++) ON(i, 0);
        } else if (card_is("$Revol04")) {                            /* (option 0 never set; 5-8 are unwritten) */
            ON(0, 1); for (int i = 5; i < 9; i++) ON(i, 0);
        } else if (card_is("$Speci00")) {
            if (!OPT(3)) ON(3, 1);                                   /* "there are some interesting rumours flying around" */
        } else if (card_is("$Other00")) {                            /* the house of the side the party serves (the rumour that showed it lasts 20 days) */
            if (s == REBELS) ON(0, 1);
            if (s == GOVT) ON(1, 1);
        } else if (card_is("$Artif00")) {                            /* (copied from the blacksmiths' card: their bit for the artisans') */
            int ag = fact_value(F_AGITATE), f5 = fact_value(5);
            if (ag == 0x20) { ON(4, 0); ON(5, 0); }
            else if (ag == 2) { ON(4, f5 & 0x20 ? 2 : 1); ON(5, f5 & 0x20 ? 2 : 1); }
        } else if (card_is("$Revol13") && OPT(1)) {                  /* the bribe (its flag got the saint's) */
            int poor = C(GF_money_cmp, 2, G_MONEY, GV_var_money1) == -1;
            ON(1, rebels_pct() >= 50 ? (poor ? 2 : 1) : 0);
            if (OPT(2) == 1) ON(2, C(GF_saints_usable, 0));
        }
#undef ON
#undef OPT
    }
    hook_call_original();
}

/* $Revol03 at the leader's house: on the decisive day, the rally or the arrest starts here */
static void revol03_hook(void)
{
    if (politics_on() && season(cur_loc())) {
        int s = side();
        g_ww(GV_party_x, (u16)(s == GOVT));                          /* (the card tells the sides apart by party_x) */
        int loc = cur_loc(), card = 0;
        if (!C(GF_event_at, 2, 0x3e, loc)) {
            if (s == REBELS && fact(F_MARCH)) card = 0xb8;
            if (s == GOVT && fact(F_ARREST)) card = 0xb7;
        }
        if (ns_unwinding) return;
        if (card) { LOG("the day: to card %#x\n", card); g_ww(GV_next_card, (u16)card); AX = (u16)card; hook_return_far(0); return; }
    }
    hook_call_original();
}

/* $Revol18 ("the revolution has started!" on the street): joining starts with the rally */
static void revol18_join_hook(void)
{
    if (!politics_on()) { hook_call_original(); return; }
    g_ww(GV_next_card, 0xb8);
    hook_return_far(0);
}

/* $Polit01, "eat your handbills": the thugs walk off, but the card never let the party go (it asked again) */
static void polit01_eat_hook(void)
{
    hook_call_original();
    if (!politics_on() || ns_unwinding) return;
    g_ww(GV_next_card, C(GF_is_daylight, 0) ? 9 : 0x0a);
}

/* the war council's options */
static void consi_review_hook(void)                                  /* 0: review the plan for taking over */
{
    if (!politics_on()) { hook_call_original(); return; }
    card_text(lord_city() ? 3 : 2);
    if (ns_unwinding) return;
    g_ww(GV_next_card, 0x70);
    hook_return_far(0);
}
static void consi_go(int who)
{
    int loc = cur_loc();
    if (day_set(loc) == -who) { card_text(9); if (ns_unwinding) return; g_ww(GV_next_card, 0x70); return; }
    if (who == REBELS) {
        int pct = rebels_pct();
        card_text(pct >= 150 ? 5 : pct >= 80 ? 6 : 7);
        if (ns_unwinding) return;
    }
    set_day(loc, who, 3);
    if (ns_unwinding) return;
    card_text(8);
    if (ns_unwinding) return;
    g_ww(GV_next_card, g_rw(GV_saved_card));
    AX = g_rw(GV_saved_card);
}
static void consi_rebels_hook(void) { if (!politics_on()) { hook_call_original(); return; } consi_go(REBELS); if (!ns_unwinding) hook_return_far(0); }
static void consi_govt_hook(void) { if (!politics_on()) { hook_call_original(); return; } consi_go(GOVT); if (!ns_unwinding) hook_return_far(0); }
static void consi_later_hook(void)                                   /* 6: not yet ("Plans in place" fits after a plan only) */
{
    if (!politics_on()) { hook_call_original(); return; }
    g_ww(GV_next_card, g_rw(GV_saved_card)); AX = g_rw(GV_saved_card);
    hook_return_far(0);
}

/* the street gossip ($Revol04): from the news ("discuss the political situation here", which went
   to the general news) and from the rumours of jobs; back to where it was asked (its own exit goes
   to saved_card, which those cards keep for their own way back) */
static int gossip_from;
static void cityn_politics_hook(void)
{
    if (!politics_on() || !season(cur_loc())) { hook_call_original(); return; }
    gossip_from = 0x66; g_ww(GV_next_card, 0xbf);
    hook_return_far(0);
}
static void speci_rumours_hook(void)
{
    if (!politics_on() || !season(cur_loc())) { hook_call_original(); return; }
    gossip_from = 0x67; g_ww(GV_next_card, 0xbf);
    hook_return_far(0);
}
static void gossip_exit_hook(void)
{
    if (!politics_on() || !gossip_from) { hook_call_original(); return; }
    C(GF_advance_hours, 1, 1);
    if (ns_unwinding) return;
    int to = gossip_from; gossip_from = 0;
    g_ww(GV_next_card, (u16)to); AX = (u16)to;
    hook_return_far(0);
}
/* $Revol04's two empty options */
static void gossip(int who)
{
    int loc = cur_loc(), set = day_set(loc), left = day_hours_left(loc), t;
    if (set == who && left > -DAY_HOURS) t = who == GOVT ? (left > 24 ? 4 : 5) : (left > 24 ? 8 : 9);
    else t = who == GOVT ? 15 : 16;
    card_text(t);
    if (!ns_unwinding) g_ww(GV_next_card, 0xbf);
}
static void gossip_govt_hook(void) { if (!politics_on()) { hook_call_original(); return; } gossip(GOVT); if (!ns_unwinding) hook_return_far(0); }
static void gossip_rebels_hook(void) { if (!politics_on()) { hook_call_original(); return; } gossip(REBELS); if (!ns_unwinding) hook_return_far(0); }

/* conflict_outcome (acting, party, winner): the news afterwards */
static void outcome_hook(void)
{
    int a = (s16)rw(SS, (u16)(SP + 4)), c = (s16)rw(SS, (u16)(SP + 8)), loc = cur_loc();
    hook_call_original();
    if (!politics_on() || ns_unwinding || loc < 0) return;
    int winner = c ? c : a;
    LOG("outcome at %d: %s won\n", loc, winner == REBELS ? "the rebels" : "the government");
    if (winner == GOVT) {                                            /* (it marked the city overthrown whoever won) */
        loc_flags_set(loc, loc_flags(loc) & ~0x80);
        C(GF_event_here, 2, 0x42, 24 * 60);                          /* "the rebellion was put down" in the news */
    }
}

/* (the margin) a season runs at loc: the party's side there (-1, 1), 2 none yet; 0 no season */
int politics_unrest_at(int loc)
{
    if (!politics_on()) return 0;
    u32 e = season_rec(loc);
    if (!e) return 0;
    int v = g_ev(e, G_EV_VALUE);
    return v == REBELS || v == GOVT ? v : 2;
}
int politics_test_start(void) { return politics_start(cur_loc()); }
/* (tests) the season here began d days earlier */
void politics_test_age(int d)
{
    u32 e = season_rec(cur_loc());
    if (!e) return;
    int y = g_ev(e, 0x10), m = g_ev(e, 0x0e), day = g_ev(e, 0x0c);
    for (int k = 0; k < d; k++) if (--day < 1) { if (--m < 0) { m = 11; y--; } day = mdays(m, y); }
    g_ev_set(e, 0x10, y); g_ev_set(e, 0x0e, m); g_ev_set(e, 0x0c, day);
    fprintf(stderr, "politics: the season is %d days old\n", season_days(cur_loc()));
}

/* ---------------------------------------------------------------- the season's cards, put right
 * (found by Arkana Mechanika, Darklands Restoration):
 *  - $Polit01, the thugs who come for a handbill poster: the main street's fight (prev_screen 6,
 *    8) falls through into the side street's (s057 1a02:0282, no break): two battles, the first
 *    one's outcome lost. The second is not fought;
 *  - $Revol00, the faction's house by day, asking around (s048 1838:03d6): at the government's
 *    house the patricians' visits (text 12) were followed by the rebels' (texts 3 or 4) again;
 *  - a potion that "makes success assured" ($Revol00 1838:0916, $Revol01 190e:06f6: their pop-ups
 *    give -1, rand(100) <= -1) never worked: it does;
 *  - $Revol01, knocking at the back door of the party's own side (s048 190e:04ee): the chance
 *    was whatever the copy left in AX; it reads "Success is assured" */
static long party_x_val(void) { return (long)((u32)g_rw(GV_party_x) | (u32)g_rw((u16)(GV_party_x + 2)) << 16); }
static int fight_active, fights, first_result;
static void polit01_battle_hook(void)       /* run_battle, inside $Polit01's fight */
{
    if (fight_active && ++fights == 2) {
        int ps = (s16)g_rw(GV_prev_screen);
        if (ps == 6 || ps == 8) { AX = (u16)first_result; hook_return_far(0); return; }
    }
    hook_call_original();
    if (fight_active && fights == 1) first_result = (s16)AX;
}
static void polit01_fight_hook(void)        /* s057 1a02:0282 */
{
    if (!politics_on()) { hook_call_original(); return; }
    fight_active = 1; fights = 0;
    hook_call_original();
    fight_active = 0;
}
static void revol00_gossip_hook(void)       /* s048 1838:03d6 (chance) */
{
    if (!politics_on() || !party_x_val()) { hook_call_original(); return; }
    int p = (s16)rw(SS, (u16)(SP + 4)), loc = cur_loc();
    C(GF_advance_hours, 1, rnd(2) + 1);
    if (rnd(100) <= p) { C(GF_card_text, 2, 2, 1); if (!ns_unwinding) C(GF_card_text, 2, 0xc, 1); }
    else C(GF_card_text, 2, 1, 1);
    if (ns_unwinding) return;
    g_ww(GV_opt_flag_0, 2);
    C(GF_event_new, 10, -2, loc, 0x5a, 0x1c, 0x1c, 0, 0, 0x5a0, 0, 2);
    int next = C(GF_is_daylight, 0) ? 0x10 : 0x11;
    g_ww(GV_next_card, (u16)next); AX = (u16)next;
    hook_return_far(0);
}
static void revol_potion_hook(void)         /* $Revol00 1838:0916, $Revol01 190e:06f6 (chance) */
{
    if (politics_on() && (s16)rw(SS, (u16)(SP + 4)) < 0) ww(SS, (u16)(SP + 4), 100);
    hook_call_original();
}
static void revol01_knock_chance_hook(void) /* s048 190e:04ee () -> (chance, who) */
{
    hook_call_original();
    if (!politics_on()) return;
    int v = fact_value(F_SIDE);
    long px = party_x_val();
    if ((v == -1 && !px) || (v == 1 && px)) { AX = 0xffff; DX = 0xfffe; }
}

/* ---------------------------------------------------------------- $Revol11, the city lord's boons
 * The lord offers one of his alchemist's formulae ($Text2) or a rare saint ($Text3), among others.
 * As MicroProse left it (the saint found by Arkana Mechanika, Darklands Restoration):
 *  - the saint (s089 18ce:02fc) is the first from the city's own that nobody in the party knows,
 *    but the routine has no return statement: the boon was saint 0;
 *  - the formula's potion (18ce:02a8) is one of six by the city; it names place 0x64.. as
 *    $NamedTwoName instead of the formula, and neither $Text2 nor $Text3 is ever set;
 *  - both are offered through the potions' and the saints' pop-ups (\10, \16), which list only
 *    what members carry or know: never the boon;
 *  - taking the formula (18ce:033c) sets formulas_known[member][the saint's number] = 7: all three
 *    formulae of the first potion.
 * Here both are plain options, named: the formula is the best of the potion's three that the
 * party's best alchemist does not know, taught to that alchemist; the saint, to the member best
 * at Religion. */
int gamedata_formula_raw(int f, char *out, int n);                  /* (ui/gamedata.cpp) */
static int boon_formula = -1, boon_saint = -1;
static int best_at(int k)
{
    int b = -1, bv = -1;
    for (int m = 0; m < 5; m++) if (gm_member_present(m) && gm_member_status(m) == 1) { int v = gm_member_skill(m, k); if (v > bv) { bv = v; b = m; } }
    return b >= 0 ? b : (s8)g_rb(G_LEADER);
}
static int knows_formula(int m, int f) { return (rb(G_FAR2E38, (u16)(0x07be + m * 22 + f / 3)) >> (f % 3)) & 1; }
static void boon_formula_hook(void)         /* s089 18ce:02a8 () -> the potion */
{
    hook_call_original();
    if (!politics_on() || ns_unwinding) return;
    int code = (s16)AX, k = code - 0x5f, m = best_at(7), g = 2;
    if (k < 0 || k >= 22) { boon_formula = -1; return; }
    while (g > 0 && knows_formula(m, 3 * k + g)) g--;
    if (knows_formula(m, 3 * k + g)) g = 2;
    boon_formula = 3 * k + g;
    char nm[48];
    if (gamedata_formula_raw(boon_formula, nm, sizeof nm)) dset(GV_var_text2, nm, 40);
    LOG("the lord's boon: formula %d (%s) for member %d\n", boon_formula, nm, m);
}
static void boon_saint_hook(void)           /* s089 18ce:02fc () -> the saint */
{
    if (!politics_on()) { hook_call_original(); return; }
    int v = (s16)C(GF_cur_loc_query, 1, 0x21) % 0x88, n = 0;
    if (v < 0) v += 0x88;
    while (C(GF_member_bit, 2, -2, v) && n++ < 0x88) { if (ns_unwinding) return; v = (v + 1) % 0x88; }
    C(GF_name_to_slot, 3, 1, v, 1);                                  /* ($NamedTwoName: text 4) */
    if (ns_unwinding) return;
    char nm[32]; int i = 0;
    for (; i < 29; i++) { u8 ch = rb(G_FAR2E38, (u16)(0x1a96 + 30 + i)); nm[i] = (char)ch; if (!ch) break; }
    nm[i] = 0;
    const char *bare = !strncmp(nm, "St. ", 4) ? nm + 4 : !strncmp(nm, "St.", 3) ? nm + 3 : nm;   /* (the texts say "Saint ...") */
    dset(GV_var_text3, bare, 40);
    for (i = 0; ; i++) { wb(G_FAR2E38, (u16)(0x1a96 + 30 + i), (u8)bare[i]); if (!bare[i]) break; }
    boon_saint = v;
    LOG("the lord's boon: saint %d (%s)\n", v, bare);
    AX = (u16)v;
    hook_return_far(0);
}
static void boon_chance_hook(void)          /* s089 18ce:0386, 0428 (the two options' chances): sure */
{
    if (!politics_on()) { hook_call_original(); return; }
    for (int i = 0; i < 80; i++) { u8 ch = g_rb((u16)(GV_str_success_assured + i)); g_wb((u16)(GV_msg_buf + i), ch); if (!ch) break; }
    AX = 0xffff; DX = 0xfffe;
    hook_return_far(0);
}
static void boon_end(int text, int hours)
{
    C(GF_card_text, 2, text, 0);
    if (ns_unwinding) return;
    C(GF_advance_hours, 1, hours);
    int next = C(GF_is_daylight, 0) ? 0x12 : 0x19;
    if (ns_unwinding) return;
    g_ww(GV_next_card, (u16)next); AX = (u16)next;
    hook_return_far(0);
}
static void boon_take_formula_hook(void)    /* s089 18ce:033c */
{
    if (!politics_on() || boon_formula < 0) { hook_call_original(); return; }
    int m = best_at(7), f = boon_formula;
    u16 a = (u16)(0x07be + m * 22 + f / 3);
    wb(G_FAR2E38, a, (u8)(rb(G_FAR2E38, a) | 1 << f % 3));
    g_ww(GV_var_chosen1, (u16)m);
    LOG("the lord's boon: member %d learns formula %d\n", m, f);
    boon_end(3, 1);
}
static void boon_take_saint_hook(void)      /* s089 18ce:03be */
{
    if (!politics_on() || boon_saint < 0) { hook_call_original(); return; }
    int m = best_at(8), s = boon_saint;
    u16 a = (u16)(0x082c + m * 0x14 + s / 8);
    wb(G_FAR2E38, a, (u8)(rb(G_FAR2E38, a) | 0x80 >> s % 8));
    g_ww(GV_var_chosen1, (u16)m);
    LOG("the lord's boon: member %d learns saint %d\n", m, s);
    boon_end(4, 2);
}

void politics_init(void)
{
    hook_install(89, 0x18ce, 0x02a8, boon_formula_hook);
    hook_install(89, 0x18ce, 0x02fc, boon_saint_hook);
    hook_install(89, 0x18ce, 0x0386, boon_chance_hook);
    hook_install(89, 0x18ce, 0x0428, boon_chance_hook);
    hook_install(89, 0x18ce, 0x033c, boon_take_formula_hook);
    hook_install(89, 0x18ce, 0x03be, boon_take_saint_hook);
    hook_install(GF_card_load, card_load_hook);
    hook_install(GF_card_options, card_options_hook);
    hook_install(GF_var_expand, var_expand_hook);
    hook_install(GF_text_expand, text_expand_hook);
    hook_install(GF_conflict_outcome_impl, outcome_hook);
    hook_install(74, 0x18f6, 0x000a, revol03_hook);
    hook_install(91, 0x18c1, 0x01fc, revol18_join_hook);
    hook_install(57, 0x1a02, 0x053c, polit01_eat_hook);
    hook_install(72, 0x1838, 0x0244, consi_review_hook);
    hook_install(72, 0x1838, 0x02d0, consi_rebels_hook);
    hook_install(72, 0x1838, 0x0318, consi_govt_hook);
    hook_install(72, 0x1838, 0x0420, consi_later_hook);
    hook_install(69, 0x19ad, 0x03e4, speci_rumours_hook);
    hook_install(69, 0x195d, 0x0358, cityn_politics_hook);
    hook_install(91, 0x18f2, 0x060e, gossip_exit_hook);
    hook_install(91, 0x18f2, 0x0496, gossip_govt_hook);
    hook_install(91, 0x18f2, 0x04d6, gossip_rebels_hook);
    hook_install(57, 0x1a02, 0x0282, polit01_fight_hook);
    hook_install(GF_run_battle, polit01_battle_hook);
    hook_install(48, 0x1838, 0x03d6, revol00_gossip_hook);
    hook_install(48, 0x1838, 0x0916, revol_potion_hook);
    hook_install(48, 0x190e, 0x06f6, revol_potion_hook);
    hook_install(48, 0x190e, 0x04ee, revol01_knock_chance_hook);
}
