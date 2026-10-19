/* offices.c - whether a town's task givers have work now, or when again (offices.h). */
#include <stdio.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "assist.h"
#include "offices.h"
#include "gamemodel.h"

static int clampi(int lo, int hi, int v) { return v < lo ? lo : v > hi ? hi : v; }
static int started(u32 e)
{
    static const u16 F[4] = { 0x10, 0x0e, 0x0c, 0x0a }, NOW[4] = { G_CUR_YEAR, G_CUR_MONTH, G_CUR_DAY, G_CUR_HOUR };
    for (int k = 0; k < 4; k++) { int a = g_ev(e, F[k]), n = (s16)g_rw(NOW[k]); if (a != n) return a < n; }
    return 1;
}
/* a started event of kind k with field f == v (and the town at +0x1c or +0x1e): the first */
static u32 find(int kind, int owner_f, int owner, int loc_f, int loc, int tag)
{
    for (int i = 0; i < G_NEVENTS; i++) {
        u32 e = g_event(i);
        if (!e || g_ev(e, G_EV_KIND) != kind || g_ev(e, (u16)owner_f) != owner || g_ev(e, (u16)loc_f) != loc) continue;
        if (tag >= 0 && g_ev(e, G_EV_TAG) != tag) continue;
        if (started(e)) return e;
    }
    return 0;
}
static int task_running(int g, int loc) { return find(8, G_EV_OWNER, g, G_EV_VALUE, loc, 3) || find(8, G_EV_OWNER, g, G_EV_VALUE, loc, 10); }
static int reward_waiting(int g, int loc) { return find(0x24, G_EV_OWNER, g, G_EV_LOC, loc, 3) || find(0x24, G_EV_OWNER, g, G_EV_LOC, loc, 10); }
static int shut(OfficeStatus *o, u32 e, const char *why)
{
    o->state = OFS_SHUT;
    o->hour = g_ev(e, 0x12); o->day = g_ev(e, 0x14); o->month = g_ev(e, 0x16); o->year = g_ev(e, 0x18);
    snprintf(o->note, sizeof o->note, "%s", why);
    return 1;
}
/* the speaker's (Charisma + Speak Common / 2): the best able member's with "the best speaks", else the leader's */
static int speaker(void)
{
    int best = -1, lead = (s8)g_rb(G_LEADER);
    for (int m = 0; m < 5; m++) {
        if (g_rb((u16)(G_MEMBER(m) + G_M_STATUS)) != 1) continue;
        if (!assist_best_leader && m != lead) continue;
        int v = (s8)g_rb((u16)(G_MEMBER(m) + G_M_ATTR_CUR + 5)) * 2 + (s8)g_rb((u16)(G_MEMBER(m) + G_M_SKILLS + 10));
        if (v > best) best = v;
    }
    return best < 0 ? 0 : best;     /* (twice Charisma + Speak Common) */
}

int office_status(int loc, int g, int hall, OfficeStatus *o)
{
    memset(o, 0, sizeof *o);
    o->chance = -1;
    u32 lr = g_location(loc);
    if (!lr) return 0;
    u16 ls = (u16)(lr >> 16), lo = (u16)lr;
    int rep = (s16)rw(ls, (u16)(lo + 0x12)), fame = (s16)g_rw(GV_party_fame);
    if (g != 10 && reward_waiting(g, loc)) { o->state = OFS_REWARD; snprintf(o->note, sizeof o->note, "You finished a task for them: your reward is waiting."); return 1; }
    if (g != 10 && task_running(g, loc)) { o->state = OFS_ON_TASK; snprintf(o->note, sizeof o->note, "You are on a task for them. They offer no other until it is done."); return 1; }
    u32 lock = g == 10 ? find(0x3c, G_EV_LOC, loc, G_EV_LOC, loc, -1) : find(7, G_EV_TYPE0, g, G_EV_LOC, loc, -1);
    switch (g) {
    case 8: case 6: case 7: {
        if (lock) return shut(o, lock, "You asked lately, or were paid a reward: no new task for three days.");
        int standing = (s16)g_rw(g == 8 ? GV_standing_fugger : g == 6 ? GV_standing_medici : GV_standing_hanse);
        if (g == 7 && (rep <= -75 || standing + rep <= -50)) { o->state = OFS_BARRED; snprintf(o->note, sizeof o->note, "The Hanse will not deal with you: your reputation in this town is too low."); return 1; }
        if (g != 7 && (rep < 0 || standing + (g == 8 && standing <= 0 ? 1 : 0) < 0)) { o->state = OFS_BARRED; snprintf(o->note, sizeof o->note, "No tasks while your reputation in this town, or your standing with the bank, is below zero."); return 1; }
        int p = g == 8 ? clampi(10, 50, rep + (s8)rb(ls, (u16)(lo + 0x15)) + fame)
              : g == 6 ? clampi(10, 50, fame + standing + rep)
              : clampi(10, 50, rep + fame > rep / 2 + standing / 2 + fame ? rep + fame : rep / 2 + standing / 2 + fame);
        o->state = OFS_ASK; o->chance = p + 1;
        snprintf(o->note, sizeof o->note, "Each time you ask (by day), %s %d%% chance of a task: %s. If none comes, or you turn it down, no other for three days.", gm_a_an(p + 1), p + 1,
                 g == 7 ? "3 in 10 are robber knights to hunt, the rest items to fetch" : "a robber knight to hunt or an item to fetch, half and half");
        return 1; }
    case 3: {
        if (!(s16)rw(ls, (u16)(lo + 0x1c))) { snprintf(o->note, sizeof o->note, "This cathedral keeps no relic, so its canon has no tasks."); return 1; }
        if (lock) return shut(o, lock, "You asked lately: the canon will not hear of it for three days.");
        int p = clampi(1, 99, rep - 50);
        o->state = OFS_ASK; o->chance = p + 1;
        snprintf(o->note, sizeof o->note, "Each time you ask the canon (by day), %s %d%% chance of a task (your reputation in this town, less 50): a robber knight to hunt or an item to fetch. If none comes, or you turn it down, no other for three days.", gm_a_an(p + 1), p + 1);
        return 1; }
    case 10: {
        if (lock) return shut(o, lock, "You sought an audience lately: for a week the chance is only 1%.");
        if (rep <= -40) { o->state = OFS_BARRED; snprintf(o->note, sizeof o->note, "The lord grants no audience while your reputation in this town is -40 or lower."); return 1; }
        int p = (rep + speaker() / 2) / 2;
        p = hall ? clampi(1, 70, p) : clampi(0, 99, p);
        if (hall && reward_waiting(10, loc)) { o->state = OFS_REWARD; snprintf(o->note, sizeof o->note, "You finished a task for the lord: your reward is waiting (an audience is certain)."); return 1; }
        o->state = OFS_ASK; o->chance = p + 1;
        if (!hall && reward_waiting(10, loc)) { snprintf(o->note, sizeof o->note, "You finished a task for the lord, but in a town without a council the game never pays the reward (a bug of the original game)."); return 1; }
        snprintf(o->note, sizeof o->note, hall ? "Each request for an audience with the council: %s %d%% chance (your reputation in this town, and the speaker's Charisma and Speak Common). An audience brings a robber knight to hunt or an item to fetch, half and half. After a request the chance is 1%% for a week."
                                               : "Each request for an audience with the lord: %s %d%% chance (your reputation in this town, and the speaker's Charisma and Speak Common). An audience brings a robber knight to hunt. After a request the chance is 1%% for a week.", gm_a_an(p + 1), p + 1);
        return 1; }
    case 0: case 4: case 5: {
        if (lock) return shut(o, lock, "No tasks for three days: at your last visit none came, one was turned down, or a reward was paid.");
        o->state = OFS_ASK; o->chance = 14;
        snprintf(o->note, sizeof o->note, "About one visit in seven brings a task: mostly an item to fetch, sometimes a robber knight to hunt. When none comes, none for three days.");
        return 1; }
    }
    return 0;
}
