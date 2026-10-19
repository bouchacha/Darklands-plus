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

/* while several days are spent at once, the hourglass of each day (drawn under this page, unseen)
   is skipped: it only shows the time passing (advance_hours has moved the clock already) */
static int many_days;
static void hourglass_hook(void)            /* pass_hours_hourglass (hours), s043 150b:2a04 */
{
    if (many_days) { hook_return_far(0); return; }
    hook_call_original();
}
static int call(u16 off, int n, int a, int b) { return n == 2 ? ns_call(36, 0x1462, off, 2, a, b) : ns_call(36, 0x1462, off, 1, a); }
#define ev_ptr g_event

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
    LodgeModel *m = &lodge_model;
    lv_mode = mode;
    ns_unwinding = 0;
    fixes_lodging(1);
    /* the original's setup (1462:0082) */
    g_wb(0x9920, 2); g_wb(0x9934, 2);                                          /* Save, Load: off */
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
            case LV_DAY: if (m->cost <= m->purse) leave = key(0x1f53, 0); break;   /* (as the button: a day paid for) */
            case LV_REST: case LV_DAYS:
                many_days = 1;
                for (int d = 0; d < (a.type == LV_DAYS ? a.a : 14) && !leave && !ns_unwinding; d++) {
                    build();
                    if (a.type == LV_REST && !m->resting) break;
                    if (m->cost > m->purse) break;                      /* (a day the purse cannot pay: the game passes no time but counts it) */
                    int before = (s16)g_rw(LV_NDAYS);
                    leave = key(0x1f53, 0);
                    if (leave || ns_unwinding || (s16)g_rw(LV_NDAYS) == before) break;   /* (something happened; or the day could not be paid) */
                    build();
                    dl_poll();
                    plat_sleep_ms(250);                                 /* (a moment to see each day) */
                }
                many_days = 0;
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

void lodgeview_init(void)
{
    lv_on = config_get_int("display.native_lodge", 1);
    hook_install(36, 0x1462, 0x0082, lodge_hook);
    hook_install(GF_pass_hours_hourglass, hourglass_hook);
}
