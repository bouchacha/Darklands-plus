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
static int barred(OfficeStatus *o, const char *why)
{
    o->state = OFS_BARRED;
    snprintf(o->note, sizeof o->note, "%s", why);
    return 1;
}
/* twice the speaker's Charisma plus Speak Common, with the contributing values */
static int speaker(int *charisma, int *speak_common)
{
    int best = -1, lead = gm_leader();
    *charisma = *speak_common = 0;
    for (int m = 0; m < 5; m++) {
        if (gm_member_status(m) != 1) continue;
        if (!assist_best_leader && m != lead) continue;
        int chr = (s8)g_rb((u16)(G_MEMBER(m) + G_M_ATTR_CUR + ATTR_CHR));
        int speak = gm_member_skill(m, SKILL_SPEAK);
        int v = chr * 2 + speak;
        if (v > best) {
            best = v;
            *charisma = chr;
            *speak_common = speak;
        }
    }
    return best < 0 ? 0 : best;
}

int office_status(int loc, int g, int hall, OfficeStatus *o)
{
    memset(o, 0, sizeof *o);
    o->chance = -1;
    u32 lr = g_location(loc);
    if (!lr) return 0;
    u16 ls = (u16)(lr >> 16), lo = (u16)lr;
    int rep = (s16)rw(ls, (u16)(lo + 0x12)), fame = (s16)g_rw(GV_party_fame);
    if (g != 10 && reward_waiting(g, loc)) { o->state = OFS_REWARD; snprintf(o->note, sizeof o->note, "You finished a quest for them: your reward is waiting."); return 1; }
    if (g != 10 && task_running(g, loc)) { o->state = OFS_ON_TASK; snprintf(o->note, sizeof o->note, "You are on a quest for them. They offer no other until it is done."); return 1; }
    u32 lock = g == 10 ? find(0x3c, G_EV_LOC, loc, G_EV_LOC, loc, -1) : find(7, G_EV_TYPE0, g, G_EV_LOC, loc, -1);
    switch (g) {
    case 8: case 6: case 7: {
        if (lock) return shut(o, lock, "You asked lately, or were paid a reward: no new quest for three days.");
        int standing = (s16)g_rw(g == 8 ? GV_standing_fugger : g == 6 ? GV_standing_medici : GV_standing_hanse);
        if (g == 7) {
            if (rep <= -75) return barred(o, "The Hanse turns you away when your reputation in this town is -75 or lower.");
            if (standing + rep <= -50) return barred(o, "Your town reputation and standing with the Hanse are too low together.");
        } else {
            if (standing < (g == 8 ? -1 : 0)) {
                const char *bank = g == 8 ? "the Fugger" : "the Medici";
                o->state = OFS_BARRED;
                snprintf(o->note, sizeof o->note, rep < 0 ? "Your town reputation and your standing with %s are both too low."
                                                          : "Your standing with %s is too low.", bank);
                return 1;
            }
            if (rep < 0) return barred(o, "Your reputation in this town is below zero.");
        }
        int fugger_bonus = g == 8 ? (s8)rb(ls, (u16)(lo + 0x15)) : 0;
        int hanse_rep = rep / 2 + standing / 2;
        int p = g == 8 ? rep + fugger_bonus : g == 6 ? rep + standing : (rep > hanse_rep ? rep : hanse_rep);
        enum { MIN_ROLL = 10, MAX_ROLL = 50 };
        o->state = OFS_ASK; o->chance = clampi(MIN_ROLL, MAX_ROLL, p + fame) + 1;
        char standing_detail[80];
        if (g == 8) snprintf(standing_detail, sizeof standing_detail, "Fugger branch bonus %+d", fugger_bonus);
        else if (g == 6) snprintf(standing_detail, sizeof standing_detail, "Medici standing %d", standing);
        else snprintf(standing_detail, sizeof standing_detail, "Hanse standing %d (used if it improves the odds)", standing);
        snprintf(o->factors, sizeof o->factors, "{Reputation:Town reputation %d} {Fame:Party fame %d}\n%s",
                 rep, fame, standing_detail);
        snprintf(o->note, sizeof o->note, "Quest chances range from %d%% to %d%%. A daytime request has %s %d%% chance of a quest. %s "
                                         "If no quest comes or you turn one down, there is no new quest for three days.",
                 MIN_ROLL + 1, MAX_ROLL + 1, gm_a_an(o->chance), o->chance,
                 g == 7 ? "Three in ten offers are to hunt a robber knight. The rest are to fetch an item."
                        : "Robber knight hunts and fetch quests are equally likely.");
        return 1; }
    case 3: {
        if (!(s16)rw(ls, (u16)(lo + 0x1c))) { snprintf(o->note, sizeof o->note, "This cathedral keeps no relic, so its canon has no quests."); return 1; }
        if (lock) return shut(o, lock, "You asked lately: the canon will offer no quest for three days.");
        int p = clampi(1, 99, rep - 50);
        o->state = OFS_ASK; o->chance = p + 1;
        snprintf(o->factors, sizeof o->factors, "{Reputation:Town reputation %d}", rep);
        snprintf(o->note, sizeof o->note, "The canon may offer a robber knight to hunt or an item to fetch by day. The chance starts at 2%% and rises with town reputation from 52 onward. If no quest comes, or you turn it down, there is no new quest for three days.");
        return 1; }
    case 10: {
        if (rep <= -40) return barred(o, "Your town reputation is -40 or lower, so you cannot request an audience.");
        int reward = reward_waiting(10, loc);
        if (hall && reward) { o->state = OFS_REWARD; snprintf(o->note, sizeof o->note, "You finished a quest for the lord: your reward is waiting (an audience is certain)."); return 1; }
        if (lock) {
            o->chance = 1;
            return shut(o, lock, "You asked for an audience lately: for a week the chance is only 1%.");
        }
        int charisma, speak_common;
        int p = (rep + speaker(&charisma, &speak_common) / 2) / 2;
        p = hall ? clampi(1, 70, p) : clampi(0, 99, p);
        o->state = OFS_ASK; o->chance = p;               /* (the number the card itself shows on the option: the same) */
        snprintf(o->factors, sizeof o->factors, "{Reputation:Town reputation %d}\n{Charisma:Speaker's Charisma %d}\n{Speak Common:Speak Common %d}",
                 rep, charisma, speak_common);
        const char *speak_weight = "Each point of the speaker's Speak Common counts half as much as a point of Charisma.";
        if (reward) {
            snprintf(o->note, sizeof o->note, "You finished a quest for the lord. In a town without a council, the fix \"The lord pays for his quests\" lets him pay at an audience. %s", speak_weight);
            return 1;
        }
        snprintf(o->note, sizeof o->note, "An ordinary audience request has %s %d%% chance of success. %s "
                                         "An audience brings %s. After a request the chance is 1%% for a week.",
                 gm_a_an(p), p, speak_weight,
                 hall ? "a robber knight to hunt or an item to fetch, half and half" : "a robber knight to hunt");
        return 1; }
    case 0: case 4: case 5: {
        if (lock) return shut(o, lock, "No quests for three days: at your last visit none came, one was turned down, or a reward was paid.");
        o->state = OFS_ASK; o->chance = 14;
        snprintf(o->note, sizeof o->note, "There is a fixed %d%% chance of a quest when you leave the trading screen. Most offers are to fetch an item, with the occasional robber knight. If none comes, there is no other offer for three days.", o->chance);
        return 1; }
    }
    return 0;
}
