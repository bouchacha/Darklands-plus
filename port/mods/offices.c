/* offices.c - whether a town's task givers have work now, or when again (offices.h). */
#include <stdio.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "assist.h"
#include "offices.h"

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
    if (g != 10 && reward_waiting(g, loc)) { o->state = OFS_REWARD; snprintf(o->note, sizeof o->note, "A task done for them: the reward waits."); return 1; }
    if (g != 10 && task_running(g, loc)) { o->state = OFS_ON_TASK; snprintf(o->note, sizeof o->note, "The party is on a task for them: no other until it is done."); return 1; }
    u32 lock = g == 10 ? find(0x3c, G_EV_LOC, loc, G_EV_LOC, loc, -1) : find(7, G_EV_TYPE0, g, G_EV_LOC, loc, -1);
    switch (g) {
    case 8: case 6: case 7: {
        if (lock) return shut(o, lock, "Asked lately (or paid): the door is shut for three days.");
        int standing = (s16)g_rw(g == 8 ? GV_standing_fugger : g == 6 ? GV_standing_medici : GV_standing_hanse);
        if (g == 7 && (rep <= -75 || standing + rep <= -50)) { o->state = OFS_BARRED; snprintf(o->note, sizeof o->note, "The Hanse shows the party out: the town's regard is too low."); return 1; }
        if (g != 7 && (rep < 0 || standing + (g == 8 && standing <= 0 ? 1 : 0) < 0)) { o->state = OFS_BARRED; snprintf(o->note, sizeof o->note, "No task while the town's regard is below nothing."); return 1; }
        int p = g == 8 ? clampi(10, 50, rep + (s8)rb(ls, (u16)(lo + 0x15)) + fame)
              : g == 6 ? clampi(10, 50, fame + standing + rep)
              : clampi(10, 50, rep + fame > rep / 2 + standing / 2 + fame ? rep + fame : rep / 2 + standing / 2 + fame);
        o->state = OFS_ASK; o->chance = p + 1;
        snprintf(o->note, sizeof o->note, "Asking (by day) brings a task %d times in 100: %s. A refusal shuts the door for three days.", p + 1,
                 g == 7 ? "three in ten a robber knight to hunt, else a thing to fetch" : "a robber knight to hunt or a thing to fetch, half and half");
        return 1; }
    case 3: {
        if (!(s16)rw(ls, (u16)(lo + 0x1c))) { snprintf(o->note, sizeof o->note, "This cathedral keeps no relic: its canon has no tasks."); return 1; }
        if (lock) return shut(o, lock, "Asked lately: the canon will not hear of it for three days.");
        int p = clampi(1, 99, rep - 50);
        o->state = OFS_ASK; o->chance = p + 1;
        snprintf(o->note, sizeof o->note, "Asking the canon (by day) brings a task %d times in 100 (the town's regard, less 50): a robber knight to hunt, or a thing to fetch; a refusal shuts it for three days.", p + 1);
        return 1; }
    case 10: {
        if (lock) return shut(o, lock, "An audience was sought lately: for a week the chance is 1 in 100.");
        if (rep <= -40) { o->state = OFS_BARRED; snprintf(o->note, sizeof o->note, "The lord grants no audience while the town's regard is -40 or lower."); return 1; }
        int p = (rep + speaker() / 2) / 2;
        p = hall ? clampi(1, 70, p) : clampi(0, 99, p);
        if (hall && reward_waiting(10, loc)) { o->state = OFS_REWARD; snprintf(o->note, sizeof o->note, "A task done for the lord: the reward waits (an audience is sure)."); return 1; }
        o->state = OFS_ASK; o->chance = p + 1;
        if (!hall && reward_waiting(10, loc)) { snprintf(o->note, sizeof o->note, "A task done for the lord waits for its reward, but in a town without a council the game never pays it (a bug of the original)."); return 1; }
        snprintf(o->note, sizeof o->note, hall ? "An audience with the council comes %d times in 100 (the town's regard, the speaker's Charisma and Speak Common): then a robber knight to hunt or a thing to fetch, half and half. Tried, it shuts for a week."
                                               : "An audience with the lord comes %d times in 100 (the town's regard, the speaker's Charisma and Speak Common): then a robber knight to hunt. Tried, it shuts for a week.", p + 1);
        return 1; }
    case 0: case 4: case 5: {
        if (lock) return shut(o, lock, "No task for three days (one was refused, or a reward paid, at the last visit).");
        o->state = OFS_ASK; o->chance = 14;
        snprintf(o->note, sizeof o->note, "Now and then, at a visit to the stall (about one in seven), a task: mostly a thing to fetch, now and then a robber knight. When none comes, none for three days.");
        return 1; }
    }
    return 0;
}
