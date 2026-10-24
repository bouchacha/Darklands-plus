/* lodgeview.c - staying at an inn, camping, as a native screen (display.native_lodge; the view is
 * ui/lodgeview.cpp).
 *
 * The original (s036 1462:0082, mode 0 an inn's room, 1 camping in the wild, 2 and 3 other stays)
 * lists the members with each one's day on the left, the seven activities on the right (Just
 * Relax, Regain Strength, Pray, Alchemy work, Earn a little money, Guard the camp, Train or study:
 * bright where the chosen member can, dim where not), the teachers of the town while the pointer
 * is on "Train or study", and Spend a day / Leave under them; one key a thing (J R P A E G T, S, L,
 * 1..5 a member, F1..F5 a sheet). Here the setup and the leaving are the original's, and every
 * choice is still made by its key handler (1462:08ec) with the member chosen as its keys choose
 * (DS:3aba); the view shows what the game holds: the activity (DS:8a18), its number (89fe), its
 * word (8a4e), the day's cost (1462:1858) and the days stayed (3abe). Training goes by the town's
 * teachers (kind 0x28 events of this place that have begun: skill, fee +0x1e, strength +0x20) in
 * their order in DS:8a38 with DS:89fa the one chosen, written here before T as the original's
 * list writes them (with the "teachers" fix's order, whether that fix is on or not). "Until
 * rested" spends days while anyone regains strength or prays (the game sets them back to
 * relaxing when full), at most 14, stopping when a day could not be paid or something happened. */
#include <stdio.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"
void plat_hold(int ms);
#include "gamemodel.h"
#include "lodgeview.h"
#include "teachers.h"

void plat_sleep_ms(int ms);
void dl_poll(void);
void fixes_lodging(int open);

LodgeModel lodge_model;
static int lv_on = 1;
static int lv_mode;
#define VECSEG ((u16)(LOADSEG + 0x9c0))
#define LV_SEL     0x3aba
#define LV_NDAYS   0x3abe
#define LV_FORMATS 0x3ac0
#define LV_TEACHER 0x89fa
#define LV_VAL     0x89fe
#define LV_ACTS    0x8a18
#define LV_LISTED  0x8a32
#define LV_RISK    0x8a34
#define LV_TLIST   0x8a38
#define LV_DETAIL  0x8a4e

int lodgeview_is_on(void) { return lv_on; }
void lodgeview_set(int on) { lv_on = on; config_set_int("display.native_lodge", on); }

/* the hours passing: the original's hourglass (advance_hours calls it before it moves the clock;
   every three hours a step with the bell, day and month, 400 ms each and a second at the end; a
   key or click skips it) would be drawn under this page, unseen, and the page would stand still
   for four seconds. Here the page shows the same steps itself, faster (150 ms; 45 ms each while
   several days go by), and a click or a key skips them (the rest of a run of days too) */
static int many_days, skip_run;
u32 plat_ticks_ms(void);
/* under any other Darklands+ page (ns_showing), the same clock is drawn over the page by the
   interface (ui/ui.cpp: hourglass_now), with the original's pace (400 ms a step) */
int hg_active, hg_hour, hg_day, hg_month;
static void hourglass_other(int steps)
{
    int h = (s16)g_rw(GV_cur_hour), day = (s16)g_rw(GV_cur_day), month = (s16)g_rw(GV_cur_month), skip = 0;
    hg_active = 1;
    ns_input_begin();
    for (int s = 0; s <= steps && !skip; s++) {
        hg_hour = h; hg_day = day; hg_month = month;
        u32 t0 = plat_ticks_ms(), wait = (u32)(s == steps ? 1000 : 400);
        while (plat_ticks_ms() - t0 < wait) {
            dl_poll();
            plat_sleep_ms(4);
            ns_input_poll();
            if (ns_in.pressed || ns_in.rclick) skip = 1;
            if (ns_call(GF_key_pressed, 0)) { ns_call(GF_get_key, 0); skip = 1; }
            if (skip || ns_unwinding) break;
        }
        if ((h += 3) >= 24) {
            h -= 24;
            if (++day > (s16)g_rw((u16)(GV_days_per_month + 2 * (month % 12)))) { day = 1; month = (month + 1) % 12; }
        }
    }
    hg_active = 0;
    ns_input_begin();
}
static void hourglass_hook(void)            /* pass_hours_hourglass (hours), s043 150b:2a04 */
{
    LodgeModel *m = &lodge_model;
    if (!lv_on || !m->open) {
        int steps = (s16)rw(SS, (u16)(SP + 4)) / 3;
        if (steps > 0 && (ns_showing() || config_get_int("display.clock", 1)) && !config_get_int("original", 0)) { hourglass_other(steps); hook_return_far(0); return; }   /* (the Darklands+ clock everywhere, or the game's under a Darklands+ page) */
        hook_call_original(); return;
    }
    int steps = (s16)rw(SS, (u16)(SP + 4)) / 3;
    if (steps <= 0 || skip_run) { hook_return_far(0); return; }
    int h = (s16)g_rw(GV_cur_hour), day = (s16)g_rw(GV_cur_day), month = (s16)g_rw(GV_cur_month);
    int per = many_days ? 45 : 150, skip = 0;
    m->passing = 1; m->pass_steps = steps;
    ns_input_begin();                                   /* (the click that spent the day is not a skip) */
    for (int s = 0; s <= steps && !skip; s++) {
        m->pass_step = s; m->pass_hour = h; m->pass_day = day; m->pass_month = month; m->seq++;
        u32 t0 = plat_ticks_ms(), wait = (u32)(s == steps ? 2 * per : per);
        while (plat_ticks_ms() - t0 < wait) {
            dl_poll();
            plat_sleep_ms(4);
            ns_input_poll();
            if (ns_in.pressed || ns_in.rclick) skip = 1;
            if (ns_call(GF_key_pressed, 0)) { ns_call(GF_get_key, 0); skip = 1; }
            if (skip || ns_unwinding) break;
        }
        if ((h += 3) >= 24) {                           /* (as the original counts the days on) */
            h -= 24;
            if (++day > (s16)g_rw((u16)(GV_days_per_month + 2 * (month % 12)))) { day = 1; month = (month + 1) % 12; }
        }
    }
    if (skip && many_days) skip_run = 1;
    m->passing = many_days && !skip_run; m->seq++;     /* (several days: the box stays up between them) */
    ns_input_begin();                                   /* (nor is the skipping click one on the page) */
    hook_return_far(0);
}
/* assist.more_jobs: the day job a member finds (s036 1462:197e, the best paid of 31 by an attribute and
   two skills) can be one of the jobs the game keeps for towns marked so in the location record
   (+0x14: mercenary, guard sergeant, bodyguard, courtier, scribe, clerk, troubadour, entertainer,
   preaching friar) - a mark no town ever gets, so the game never offered them. Here, while the
   job is chosen, a town counts as marked by what it has: an arsenal or a fortress (0x01: soldiers),
   a fortress (0x04: a lord's household), a town hall (0x08: the council's clerks and players), a
   city of size 6 or more (0x10: a great house's scribes and guards), size 4 or more (0x20: crowds
   to entertain), a monastery (0x40: preaching friars). */
/* the day jobs' names, made to say what the job is by the skills it pays for (the pay and the choice
   are the game's own, unchanged): a job the table has twice (a plain one and a better-paid one where
   the town has a fortress, a town hall, a great house, a monastery) is told apart by that place.
   Not in the original game. */
static void job_rename(int who, u8 marks)
{
    if (who < 0 || who > 4 || config_get_int("original", 0)) return;
    u16 o = (u16)(0x8a4e + who * 20);
    char nm[20]; g_str(G_DGROUP, o, nm, sizeof nm);
    static const struct { const char *was; u8 mark; const char *with, *without; } R[] = {
        { "Bodyguard", 0x04, "Castle Guard", "Bodyguard" },          /* Endurance, edged and impact weapons */
        { "Clerk", 0x08, "Town Clerk", "Clerk" },                    /* Intelligence, speaking and writing */
        { "Scribe", 0x10, "Notary", "Scribe" },                      /* Intelligence, Latin and writing */
        { "Servant", 0x04, "Castle Servant", "Servant" },            /* Perception, Streetwise, Virtue */
        { "Courtier", 0x04, "Castle Steward", "House Steward" },     /* the same, better paid: running a household */
        { "Preaching Friar", 0x40, "Preaching Friar", "Street Preacher" },   /* Charisma, speaking, Religion or Virtue */
        { "Guard Sergeant", 0, "Mounted Sergeant", 0 },              /* Strength, polearms, Riding */
        { "Taskmaster", 0, "Overseer", 0 },                          /* Perception, Streetwise, flails (the whip) */
        { "Troubador", 0, "Juggler", 0 },                            /* Agility, speaking, thrown weapons */
        { "Entertainer", 0, "Street Performer", 0 },                 /* the same, in a crowd */
        { "Mendicant Friar", 0, "Hospital Helper", 0 },              /* Perception, Healing, Virtue */
        { "Militiaman", 0, "Town Militia", 0 },                      /* Strength, edged weapons, polearms */
    };
    for (size_t i = 0; i < sizeof R / sizeof R[0]; i++) {
        if (strcmp(nm, R[i].was)) continue;
        const char *t = !R[i].mark || (marks & R[i].mark) || !R[i].without ? R[i].with : R[i].without;
        for (int k = 0; k < 20; k++) { g_wb((u16)(o + k), (u8)t[k]); if (!t[k]) break; }
        return;
    }
}
static void job_hook(void)                   /* s036 1462:197e (who) */
{
    int who = (s16)rw(SS, (u16)(SP + 4));
    int cur = (s16)g_rw(G_CUR_LOC);
    u32 lr = cur >= 0 ? g_location(cur) : 0;
    if (!lr || !config_get_int("assist.more_jobs", 1) || config_get_int("original", 0)) {
        u8 marks = lr ? rb((u16)(lr >> 16), (u16)((u16)lr + 0x14)) : 0;
        hook_call_original(); job_rename(who, marks); return;
    }
    int citynav_town_facts(int loc, int *size), size = 0, b = citynav_town_facts(cur, &size);
    u16 ls = (u16)(lr >> 16), lo = (u16)((u16)lr + 0x14);
    u8 was = rb(ls, lo), add = 0;
    if (b & ((1 << 5) | (1 << 2))) add |= 0x01;
    if (b & (1 << 2)) add |= 0x04;
    if (b & (1 << 1)) add |= 0x08;
    if (size >= 6) add |= 0x10;
    if (size >= 4) add |= 0x20;
    if (b & (1 << 7)) add |= 0x40;
    wb(ls, lo, (u8)(was | add));
    hook_call_original();
    wb(ls, lo, was);
    job_rename(who, (u8)(was | add));
}
static int call(u16 off, int n, int a, int b) { return n == 2 ? ns_call(36, 0x1462, off, 2, a, b) : ns_call(36, 0x1462, off, 1, a); }
#define ev_ptr g_event

static long days_of(int y, int mo, int d)       /* (a day count: month 0-11, the Julian calendar) */
{
    static const int CUM[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    if (mo < 0 || mo > 11) mo = 0;
    return (long)y * 365 + y / 4 + CUM[mo] + (y % 4 == 0 && mo > 1) + d;
}
/* the town's teachers that have begun, in the events' order */
static void teachers(LodgeModel *m)
{
    int loc = (s16)g_rw(GV_cur_loc);
    m->nteach = 0;
    for (int i = 0; i < 300 && m->nteach < LV_TEACHERS; i++) {
        u32 e = ev_ptr(i);
        if (!e) continue;
        u16 s = (u16)(e >> 16), o = (u16)e;
        if ((s16)rw(s, (u16)(o + 0x22)) != 0x28 || (s16)rw(s, (u16)(o + 0x1c)) != loc) continue;
        if (!ns_call(29, 0x0e76, 0x3180, 1, i)) continue;                 /* event_has_started */
        m->t[m->nteach].skill = (s16)rw(s, o);
        m->t[m->nteach].fee = (s16)rw(s, (u16)(o + 0x1e));
        m->t[m->nteach].val = (s16)rw(s, (u16)(o + 0x20));
        {   /* the days left of its week: its end (+0x12 hour, +0x14 day, +0x16 month, +0x18 year) from now */
            long end = days_of((s16)rw(s, (u16)(o + 0x18)), (s16)rw(s, (u16)(o + 0x16)), (s16)rw(s, (u16)(o + 0x14)));
            long now = days_of((s16)g_rw(G_CUR_YEAR), (s16)g_rw(G_CUR_MONTH), (s16)g_rw(G_CUR_DAY));
            m->t[m->nteach].days_left = (int)(end - now);
        }
        m->t[m->nteach].used = teach_best_here(m->t[m->nteach].skill) == i;
        m->nteach++;
    }
}
static int fee_of(int skill)
{
    int e = ns_call(GF_event_find, 6, 0x28, skill, (s16)g_rw(GV_cur_loc), -1, -1, -1);
    if (e < 0) return 0;
    u32 p = ev_ptr(e);
    return p ? (s16)rw((u16)(p >> 16), (u16)((u16)p + 0x1e)) : 0;
}

/* the game's line for a member's day: its format (DS:3ac0) with the number and the word */
static void day_line(char *out, int n, int act, int val, const char *detail)
{
    char fmt[48];
    u16 p = g_rw((u16)(LV_FORMATS + 2 * act));
    g_str(G_DGROUP, p, fmt, sizeof fmt);
    int o = 0;
    for (const char *f = fmt; *f && o < n - 1; f++) {
        if (*f == '%') {
            const char *g = f + 1;
            while (*g >= '0' && *g <= '9') g++;
            char tmp[32] = "";
            if (*g == 'd') { snprintf(tmp, sizeof tmp, "%d", val); f = g; }
            else if (*g == 's') { snprintf(tmp, sizeof tmp, "%s", detail); f = g; }
            for (char *t = tmp; *t && o < n - 1; t++) out[o++] = *t;
            continue;
        }
        out[o++] = *f;
    }
    out[o] = 0;
}

static int day_changed(const LodgeModel *was, const LodgeModel *now, int until_rested);
static void build(void)
{
    LodgeModel *m = &lodge_model;
    m->mode = lv_mode;
    m->sel = (s16)g_rw(LV_SEL);
    m->days = (s16)g_rw(LV_NDAYS);
    m->risk = (s16)g_rw(LV_RISK) / 5;
    teachers(m);
    m->resting = 0;
    long fees = 0, earn = 0;
    for (int i = 0; i < 5; i++) {
        LodgeMember *p = &m->m[i];
        memset(p, 0, sizeof *p);
        p->present = g_rb(G_MEMBER(i)) == 1;
        if (!p->present) continue;
        gm_member_name(i, p->name, sizeof p->name);
        p->act = (s16)g_rw((u16)(LV_ACTS + 2 * i));
        p->val = (s16)g_rw((u16)(LV_VAL + 2 * i));
        g_str(G_DGROUP, (u16)(LV_DETAIL + 20 * i), p->detail, sizeof p->detail);
        gm_member_attr(i, 1, &p->str, &p->str_max);
        gm_member_attr(i, 6, &p->df, &p->df_max);
        gm_member_attr(i, 0, &p->end, &p->end_max);
        p->skill = -1;
        if (p->act >= 0 && p->act <= 6) day_line(p->line, sizeof p->line, p->act, p->val, p->detail);
        if (p->act == 6) { p->skill = p->val; fees += fee_of(p->val); }
        if (p->act == 4) earn += p->val;
        if (p->act == 1 || p->act == 2) m->resting = 1;
        p->can[0] = 1;
        p->can[1] = p->str < p->str_max;
        p->can[2] = p->df < p->df_max;
        p->can[3] = call(0x1e80, 1, i, 0) != 0;                          /* knows a formula */
        p->can[4] = lv_mode == 0;
        p->can[5] = lv_mode == 1;
        p->can[6] = m->nteach > 0;
    }
    { u16 lo = (u16)ns_call(GF_money_pfennigs, 1, GV_party_money); m->purse = (long)(((u32)(u16)ns_call_dx() << 16) | lo); }
    m->cost = (s16)call(0x1858, 1, lv_mode, 0);
    m->lodging = m->cost - fees + earn;
    m->place[0] = 0;
    int loc = (s16)g_rw(GV_cur_loc);
    u32 r = loc >= 0 ? g_location(loc) : 0;
    if (r) { char nm[24]; for (int k = 0; k < 20; k++) nm[k] = (char)rb((u16)(r >> 16), (u16)((u16)r + 0x26 + k)); nm[20] = 0; snprintf(m->place, sizeof m->place, "%s", nm); }
    m->seq++;
}

/* the original's key handler; 0x11b: the stay ends */
static int key(int k, int nested)
{
    if (nested) ns_nested = 1;
    int r = call(0x08ec, 2, k, lv_mode);
    ns_nested = 0;
    if (nested) ns_input_begin();
    return (u16)r == 0x11b;
}
static int select_member(int i)
{
    static const int K[5] = { 0x0231, 0x0332, 0x0433, 0x0534, 0x0635 };
    if (i < 0 || i >= 5 || g_rb(G_MEMBER(i)) != 1) return 0;
    if ((s16)g_rw(LV_SEL) != i) key(K[i], 0);
    return 1;
}

static void lodge_hook(void)               /* the lodging screen (mode), s036 1462:0082 */
{
    int mode = (s16)rw(SS, (u16)(SP + 4));
    if (!lv_on || !ns_active()) { hook_call_original(); return; }
    plat_hold(600);                              /* (the old screen drawn while it is set up: not shown) */
    LodgeModel *m = &lodge_model;
    lv_mode = mode;
    ns_unwinding = 0;
    fixes_lodging(1);
    /* the original's setup (1462:0082) */
    g_wb(GV_font_flag_a, 2); g_wb(GV_font_flag_b, 2);                                          /* Save, Load: off */
    g_ww(GV_saved_back_font, g_rw(GV_back_win_font)); g_ww(GV_saved_screen_font, g_rw(GV_screen_win_font));
    g_ww(0x9c3d, (u16)ns_call_ptr(VECSEG, 0x18f1, 1, 5));
    ns_call(-1, 0x047c, 0x1320, 4, 6, 0, 0, 0);
    if (mode == 1) ns_call_ptr(VECSEG, 0x18f1, 1, 0x15);
    else ns_call_ptr(VECSEG, 0x18f1, 1, ns_call(GF_rand_mod, 1, 2) ? 9 : 10);
    g_wb(GV_screen_kind, 3);
    g_ww(LV_LISTED, 0); g_ww(LV_TEACHER, 0);
    teachers(m);
    for (int t = 0; t < m->nteach; t++) g_ww((u16)(LV_TLIST + 2 * t), (u16)m->t[t].skill);
    g_ww(LV_NDAYS, 0);
    for (int i = 0; i < 5; i++) {
        g_ww((u16)(LV_ACTS + 2 * i), 0);
        if (g_rb(G_MEMBER(i))) g_ww((u16)(LV_VAL + 2 * i), (u16)((s8)g_rb((u16)(G_MEMBER(i) + 0x64)) / 15));
    }
    ns_call(GF_screen_hold, 0);
    call(0x0862, 1, mode, 0);
    call(0x0872, 1, mode, 0);
    ns_call(GF_screen_release, 0);
    if (ns_unwinding) { fixes_lodging(0); return; }
    build();
    /* assist.inn_earn: a day of Just Relax refills Endurance and nothing else; one already at full
       Endurance earns a little money instead (an inn in a town), as the player would choose */
    if (mode == 0 && config_get_int("assist.inn_earn", 1) && !config_get_int("original", 0)) {
        int any = 0, sel0 = (s16)g_rw(LV_SEL);
        for (int i = 0; i < 5 && !ns_unwinding; i++) {
            const LodgeMember *p = &m->m[i];
            if (p->present && p->act == 0 && p->can[4] && p->end >= p->end_max && select_member(i)) { key(0x1245, 0); any = 1; }
        }
        if (any && !ns_unwinding) { select_member(sel0 >= 0 ? sel0 : 0); build(); }
        if (ns_unwinding) { fixes_lodging(0); return; }
    }
    m->open = 1;
    fprintf(stderr, "lodging: open (mode %d, %d teachers, a day %ld pf)\n", mode, m->nteach, m->cost);
    ns_enter(NS_LODGE);
    ns_clear();
    ns_input_begin();
    int leave = 0;
    while (!leave) {
        if (!ns_round()) break;
        ns_call(-1, 0x047c, 0x1320, 4, 6, 0, 0, 0);            /* (the clicks so far taken, as the original's loop does) */
        int cx, cy;
        if (ns_in.click && ns_in.cx < 0x3c) {                 /* a portrait: the member's sheet, as the original */
            ns_take_click(&cx, &cy);
            int i = cy / 40;
            if (i >= 0 && i < 5 && g_rb(G_MEMBER(i))) {
                ns_nested = 1;
                ns_call_ptr(VECSEG, 0x1fdb, 1, i);
                ns_nested = 0;
                if (ns_unwinding) break;
                g_ww(GV_party_icons_dirty, 1);
                ns_input_begin();
            }
        }
        if (g_rw(GV_party_icons_dirty)) {
            g_ww(GV_party_icons_dirty, 0);
            ns_call(GF_party_panel_frames, 1, 0x100); ns_call(GF_party_panel_rows, 1, 0x100);
            build();
        }
        if (ns_call(GF_key_pressed, 0)) {                     /* the original's keys, all of them */
            int k = ns_get_key();
            if (k == 0x11b) leave = 1;
            else if (k) {
                int nested = (k >> 8) >= 0x3b && (k >> 8) <= 0x3f;          /* F1..F5: a sheet */
                nested |= k == 0x1e41 || k == 0x1e61;                       /* A: the formulae */
                if (k == 0x1454 || k == 0x1474)                             /* T: the teachers in order */
                    for (int t = 0; t < m->nteach; t++) g_ww((u16)(LV_TLIST + 2 * t), (u16)m->t[t].skill);
                if (key(k, nested)) leave = 1;
            }
            if (ns_unwinding) break;
            build();
        }
        NsAction a;
        while (!leave && !ns_unwinding && ns_pop(&a)) {
            static const int ACT[6] = { 0x244a, 0x1352, 0x1950, 0x1e41, 0x1245, 0x2247 };
            switch (a.type) {
            case LV_SELECT: select_member(a.a); break;
            case LV_ACT:
                if (a.a >= 0 && a.a < 6 && select_member(a.b)) leave = key(ACT[a.a], a.a == 3);
                break;
            case LV_TEACH:
                if (a.a >= 0 && a.a < m->nteach && !teach_capped(a.b, m->t[a.a].skill, m->t[a.a].val) && select_member(a.b)) {   /* (nothing left to learn from one: not chosen) */
                    for (int t = 0; t < m->nteach; t++) g_ww((u16)(LV_TLIST + 2 * t), (u16)m->t[t].skill);
                    g_ww(LV_TEACHER, (u16)a.a);
                    leave = key(0x1454, 0);
                }
                break;
            case LV_DAY: if (m->cost <= m->purse) { m->pass_n = m->pass_of = 0; leave = key(0x1f53, 0); } break;   /* (as the button: a day paid for) */
            case LV_REST: case LV_DAYS:
                if (a.type == LV_REST) {                                /* (those short of strength who only relax regain it) */
                    for (int i = 0; i < 5 && !leave && !ns_unwinding; i++) {
                        const LodgeMember *p = &m->m[i];
                        if (p->present && p->act == 0 && p->str < p->str_max && p->can[1] && select_member(i)) leave = key(ACT[1], 0);
                    }
                    build();
                }
                many_days = 1; skip_run = 0;
                ns_input_begin();
                for (int d = 0; d < (a.type == LV_DAYS ? a.a : 14) && !leave && !ns_unwinding; d++) {
                    build();
                    if (a.type == LV_REST && !m->resting) break;
                    if (m->cost > m->purse) break;                      /* (a day the purse cannot pay: the game passes no time but counts it) */
                    int before = (s16)g_rw(LV_NDAYS);
                    LodgeModel was = *m;                                /* (to see what the day changed) */
                    m->pass_n = d + 1; m->pass_of = a.type == LV_DAYS ? a.a : 0;
                    leave = key(0x1f53, 0);
                    if (leave || ns_unwinding || (s16)g_rw(LV_NDAYS) == before) break;   /* (something happened; or the day could not be paid) */
                    build();
                    if (d + 1 < (a.type == LV_DAYS ? a.a : 14) && day_changed(&was, m, a.type == LV_REST)) break;   /* (the days stop where a choice would be made anew) */
                    for (int w = 0; w < 30 && !skip_run; w++) {         /* (a moment to see each day: the hours went by on the page) */
                        dl_poll(); plat_sleep_ms(4);
                        ns_input_poll();
                        if (ns_in.pressed || ns_in.rclick) skip_run = 1;
                    }
                    if (skip_run) m->passing = 0;
                }
                many_days = 0; skip_run = 0; m->passing = 0; m->pass_n = m->pass_of = 0; m->seq++;
                ns_input_begin();
                break;
            case LV_SHEET: if (a.a >= 0 && a.a < 5) leave = key(0x3b00 + (a.a << 8), 1); break;
            case LV_LEAVE: leave = 1; break;
            }
            if (ns_unwinding) break;
            build();
        }
    }
    m->open = 0;
    ns_leave();
    fixes_lodging(0);
    if (ns_unwinding) { fprintf(stderr, "lodging: closed (unwound)\n"); return; }
    call(0x082c, 1, mode, 0);                                   /* the original's leaving */
    fprintf(stderr, "lodging: closed (%d days)\n", (s16)g_rw(LV_NDAYS));
    hook_return_far(0);
}

/* a run of days (A week, Until rested) stops when the day's choices no longer hold: a member's
   activity changed by the game (a teacher gone, nothing left to do), a teacher in use leaving, or
   someone regaining Strength or praying now full. Said in a notice. 1: stop */
static int day_changed(const LodgeModel *was, const LodgeModel *now, int until_rested)
{
    void ui_notice(const char *);
    char msg[160]; msg[0] = 0;
    for (int i = 0; i < 5 && !msg[0]; i++) {
        const LodgeMember *a = &was->m[i], *b = &now->m[i];
        if (!a->present || !b->present) continue;
        char nm[24]; snprintf(nm, sizeof nm, "%s", b->name);
        if (a->act == 6 && a->skill >= 0) {
            int there = 0;
            for (int t = 0; t < now->nteach; t++) if (now->t[t].skill == a->skill && now->t[t].days_left > 0) there = 1;
            if (!there || b->act != 6) { snprintf(msg, sizeof msg, "The days stop: %s's teacher has gone.", nm); break; }
        }
        if (a->act != b->act) { snprintf(msg, sizeof msg, "The days stop: what %s does each day has changed.", nm); break; }
        if (until_rested) continue;                             /* (Until rested runs on while anyone still rests) */
        if (b->act == 1 && a->str < a->str_max && b->str >= b->str_max) snprintf(msg, sizeof msg, "The days stop: %s has regained all Strength.", nm);
        if (b->act == 2 && a->df < a->df_max && b->df >= b->df_max) snprintf(msg, sizeof msg, "The days stop: %s's Divine Favor is full.", nm);
    }
    if (!msg[0]) return 0;
    fprintf(stderr, "lodging: %s\n", msg);
    ui_notice(msg);
    return 1;
}

void lodgeview_init(void)
{
    lv_on = config_get_int("display.native_lodge", 1);
    hook_install(36, 0x1462, 0x0082, lodge_hook);
    hook_install(GF_pass_hours_hourglass, hourglass_hook);
    hook_install(36, 0x1462, 0x197e, job_hook);
}
