/* losses.c - what is lost for good is told in a window to click through (the game waits):
 *  - an attribute's maximum or a skill lowered, by whatever does it (a card, a curse ...): the
 *    members' maxima and skills are watched from one moment to the next. A saint's or a potion's
 *    boost or lowering for a while (events 0x48 / 0x49, which change the attribute or skill and
 *    give it back) is not counted; a game loaded, a member made, gone or come, the editor, a
 *    battle (whose fighting changes the records for its length) start the watch afresh;
 *  - armour worn down by acid or fire (s019 1ef8:0968), or ruined by it;
 *  - the years' toll is aging.c's own (it tells it itself).
 * Not counted: endurance and strength (wounds heal), money, things sold, dropped or taken (the
 * card or screen that does it says so). */
#include <stdio.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../runtime/dispatch.h"
#include "gamemodel.h"

void ui_alert(const char *title, const char *body, const char *foot);
void ui_text_utf8(const char *game, char *out, int n);
int ui_is_open(void);
u32 plat_ticks_ms(void);

int losses_on(void) { return config_get_int("assist.loss_alerts", 1) && !config_get_int("original", 0); }

static struct { char name[24]; s8 max[7]; s8 skill[19]; int have; } snap[5];
static int snap_seed;

/* the temporary part of attribute a (0x48) or skill k (0x49) of member m: the boosts and lowerings running */
static int temp_of(int kind, int m, int idx)
{
    int t = 0;
    for (int i = 0; i < G_NEVENTS; i++) {
        u32 e = g_event(i); if (!e) continue;
        u16 s = (u16)(e >> 16), o = (u16)e;
        if ((s16)rw(s, (u16)(o + 0x22)) == kind && (s16)rw(s, (u16)(o + 0x1a)) == m && (s16)rw(s, o) == idx) t += (s16)rw(s, (u16)(o + 0x1e));
    }
    return t;
}
static void take(int m)
{
    gm_member_name(m, snap[m].name, sizeof snap[m].name);
    for (int a = 0; a < 7; a++) snap[m].max[a] = (s8)g_rb((u16)(G_MEMBER(m) + G_M_ATTR_MAX + a));
    for (int k = 0; k < 19; k++) snap[m].skill[k] = (s8)(g_rb((u16)(G_MEMBER(m) + G_M_SKILLS + k)) - temp_of(0x49, m, k));
    snap[m].have = gm_member_present(m);
}
/* (aging.c lowered member m's attributes and told it: taken as they are now) */
void losses_retake(int m) { if (m >= 0 && m < 5) take(m); }

void losses_battle_frame(void);
void losses_frame(void)
{
    losses_battle_frame();
    extern unsigned dos_save_read_ms;
    static unsigned last_read; static int grace;
    if (dos_save_read_ms != last_read) { last_read = dos_save_read_ms; grace = 300; }
    if (grace > 0) grace--;
    int seed = (s16)g_rw(G_PARTY_SEED);
    int gm = game_mode();
    if (gm == GM_BATTLE) return;                                     /* (the watch waits for its end) */
    int fresh = !losses_on() || grace > 0 || gm == GM_OTHER || ovl_loaded[37] || ovl_loaded[38] || ui_is_open() || seed != snap_seed;
    if (fresh) { for (int m = 0; m < 5; m++) take(m); snap_seed = seed; return; }
    char body[1600]; size_t len = 0; body[0] = 0;
    for (int m = 0; m < 5; m++) {
        char nm[24]; gm_member_name(m, nm, sizeof nm);
        int present = gm_member_present(m);
        if (!present || !snap[m].have || strcmp(nm, snap[m].name)) { take(m); continue; }   /* (a member gone, come or moved) */
        char part[400]; size_t pl = 0; part[0] = 0;
        for (int a = 0; a < 7; a++) {
            int v = (s8)g_rb((u16)(G_MEMBER(m) + G_M_ATTR_MAX + a));
            if (v < snap[m].max[a]) { pl += (size_t)snprintf(part + pl, sizeof part - pl, "%s%s %d (was %d)", pl ? ", " : "", gm_attr_name(a), v, snap[m].max[a]); if (pl >= sizeof part) pl = sizeof part - 1; }
        }
        for (int k = 0; k < 19; k++) {
            int v = (s8)(g_rb((u16)(G_MEMBER(m) + G_M_SKILLS + k)) - temp_of(0x49, m, k));
            if (v < snap[m].skill[k]) { pl += (size_t)snprintf(part + pl, sizeof part - pl, "%s%s %d (was %d)", pl ? ", " : "", gm_skill_name(k), v, snap[m].skill[k]); if (pl >= sizeof part) pl = sizeof part - 1; }
        }
        take(m);
        if (!part[0]) continue;
        char nu[80]; ui_text_utf8(nm, nu, sizeof nu);
        len += (size_t)snprintf(body + len, sizeof body - len, "%s%s: %s.", len ? "\n" : "", nu, part);
        if (len >= sizeof body) len = sizeof body - 1;
    }
    if (body[0]) ui_alert("Lost for good", body, "The maximum of an attribute, or a skill, is lower now, and will not come back by itself.");
}

/* ---------------------------------------------------------------- armour worn down */
#define CB_NITEMS(i) ((u16)(GV_combatants + (i) * 0x80 + 0x7e))
static char worn[1200]; static size_t worn_len;
static void wear_hook(void)                  /* s019 1ef8:0968 (i, type, quality, amount): wear an armour down */
{
    int i = (s16)rw(SS, (u16)(SP + 4)), type = rw(SS, (u16)(SP + 6)) & 0xff, q = rw(SS, (u16)(SP + 8)) & 0xff, amt = (s16)rw(SS, (u16)(SP + 10));
    int id = -1;
    if (i >= 0 && i < 5) {
        int n = (s8)g_rb(CB_NITEMS(i));
        for (int k = 0; k < n && k < 64; k++)
            if (rb(G_FAR2E38, (u16)(G_ITEM(i, k) + 2)) == type && rb(G_FAR2E38, (u16)(G_ITEM(i, k) + 3)) == q) { id = rw(G_FAR2E38, G_ITEM(i, k)); break; }
    }
    hook_call_original();
    if (id < 0 || amt <= 0 || !losses_on()) return;
    char nm[24], nu[80], it[40], iu[80];
    gm_member_name(i, nm, sizeof nm); ui_text_utf8(nm, nu, sizeof nu);
    gm_itemdef_name(id, it, sizeof it); ui_text_utf8(it, iu, sizeof iu);
    int nq = q - amt;
    worn_len += (size_t)snprintf(worn + worn_len, sizeof worn - worn_len, "%s%s's %s: ", worn_len ? "\n" : "", nu, iu);
    if (worn_len >= sizeof worn) worn_len = sizeof worn - 1;
    if (nq <= 0) worn_len += (size_t)snprintf(worn + worn_len, sizeof worn - worn_len, "worn to nothing and gone (it was %dq).", q);
    else worn_len += (size_t)snprintf(worn + worn_len, sizeof worn - worn_len, "now %dq (was %dq).", nq, q);
    if (worn_len >= sizeof worn) worn_len = sizeof worn - 1;
}
/* told once a frame, all that wore down together (a splash of acid on several members) */
void losses_battle_frame(void)
{
    if (!worn_len) return;
    ui_alert("Armour ruined", worn, "Acid and fire wear armour down for good: a smith's work cannot mend it; a better piece must be bought.");
    worn_len = 0; worn[0] = 0;
}

void losses_init(void) { hook_install(19, 0x1ef8, 0x0968, wear_hook); }
