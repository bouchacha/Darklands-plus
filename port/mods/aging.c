/* aging.c - the years take their toll in play, as they do while a character is made.
 *
 * The game ages characters only at creation (s037 18c8:356c): each time a career step brings the
 * age to 30 + 5k, row k of the table at 290e:3521 (End, Str, Agl, Per, Int, Chr) is added to the
 * attributes, so that by 70 a character has lost 25 Endurance, 21 Strength and Agility, 10
 * Perception and Charisma and 5 Intelligence. In play the game counts birthdays (s029 0e76:281c, at
 * every month's start: a member's birth month is the one with (party seed + month) % 12 == slot)
 * and nothing more.
 *
 * Here the same table is spread over the years: what is due at an age is the creation's total
 * up to the last fifth year plus the share of the next row the months since have reached; at
 * every month's start each member loses what the month added (whole points, when the share
 * crosses one). It needs no record of its own: what is due from one month to the next follows
 * from the age alone, so loading an older save simply ages from there. The losses are lost for
 * good (the maximum and the attribute now), and told in a window to click through. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "gamemodel.h"

#include "../ui/alerts.h"
void ui_text_utf8(const char *game, char *out, int n);     /* (ui: the game's characters as UTF-8) */
void losses_retake(int m);                                  /* (losses.c: told here, not there too) */

#define T290E ((u16)(LOADSEG + 0x290e))
static int row(int k, int a) { if (k > 8) k = 8; return (s8)rb(T290E, (u16)(0x3521 + 6 * k + a)); }   /* (negative: a loss) */

/* the loss of attribute a due at an age of `months` months, as creation's table spread over the years */
static double due(int a, int months)
{
    double y = months / 12.0;
    if (y <= 30) return 0;
    double t = (y - 30) / 5; int k = (int)t;
    double l = 0;
    for (int j = 1; j <= k; j++) l -= row(j, a);
    l -= (t - k) * row(k + 1, a);
    return l;
}
static int age_months(int m)
{
    int years = (s16)g_rw((u16)(G_MEMBER(m) - 2)), month = (s16)g_rw(G_CUR_MONTH), seed = (s16)g_rw(G_PARTY_SEED), b = 0;
    for (int k = 0; k < 12; k++) if ((u16)(seed + k) % 12 == (unsigned)m) { b = k; break; }
    return years * 12 + (month - b + 12) % 12;
}

int aging_on(void) { return config_get_int("rules.aging", 1) && !config_get_int("original", 0); }

/* a month has begun (or, in tests, `n` of them): each member loses what the months added */
void aging_month(int n)
{
    char body[1600]; size_t len = 0; body[0] = 0;
    int any = 0;
    UiAlertRow rows[5]; int nr = 0;
    for (int m = 0; m < 5; m++) {
        int st = gm_member_status(m);
        if (!gm_member_present(m) || (st != 1 && st != 2)) continue;
        int A = age_months(m);
        UiAlertRow *row = &rows[nr]; row->n = 0; row->age = A / 12; gm_member_name(m, row->name, sizeof row->name);
        char part[300]; size_t pl = 0; part[0] = 0;
        for (int a = 0; a < 6; a++) {
            int d = (int)floor(due(a, A) + 1e-9) - (int)floor(due(a, A - n) + 1e-9);
            if (d <= 0) continue;
            u16 mx = (u16)(G_MEMBER(m) + G_M_ATTR_MAX + a), cu = (u16)(G_MEMBER(m) + G_M_ATTR_CUR + a);
            int was = (s8)g_rb(mx), now = was - d; if (now < 1) now = 1;
            if (now == was) continue;
            g_wb(mx, (u8)now);
            int c = (s8)g_rb(cu); if (c > 0) { c -= was - now; if (c < 1) c = 1; g_wb(cu, (u8)c); }
            pl += (size_t)snprintf(part + pl, sizeof part - pl, "%s%s %d (was %d)", pl ? ", " : "", gm_attr_name(a), now, was);
            if (pl >= sizeof part) pl = sizeof part - 1;
            row->ch[row->n].key = a; row->ch[row->n].was = was; row->ch[row->n].now = now; row->n++;
        }
        losses_retake(m);
        if (!part[0]) continue;
        nr++;
        char nm[40], nu[80]; gm_member_name(m, nm, sizeof nm); ui_text_utf8(nm, nu, sizeof nu);
        len += (size_t)snprintf(body + len, sizeof body - len, "%s%s, now %d: %s.", len ? "\n" : "", nu, A / 12, part);
        if (len >= sizeof body) len = sizeof body - 1;
        any = 1;
    }
    if (getenv("DL_AGING_LOG")) fprintf(stderr, "aging: %d month(s): %s\n", n, any ? body : "nothing");
    if (any) ui_alert_rows("The years take their toll", body, rows, nr,
                           "Beside each name, the age (the hourglass); then each attribute's maximum lowered: what it was, and what it is now. "
                           "These losses are permanent, as with aging at character creation: from age 30, a little each month, and more as the years go by. "
                           "(Aging during play is a Darklands+ switch: F12 > Settings > The party and its items.)");
}

static void month_hook(void)                 /* s029 0e76:281c (months): a month has begun; birthdays counted */
{
    hook_call_original();
    if (aging_on()) aging_month(1);
}

void aging_init(void) { hook_install(29, 0x0e76, 0x281c, month_hook); }
