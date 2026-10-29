/* styles.c - each party member's way of fighting: what they do in a battle while the player gives them no
 * order (Improved combat, mods/ways.c carries it out; an order of the player's is carried out, then they go
 * back to their way). Chosen on the member's box in the party panel (ui/partypanel.cpp) and kept by the
 * character's name (battle.style.<name>), so it holds from battle to battle and save to save. Until one is
 * chosen: Skirmisher, for everyone (one without a missile weapon waits while the others shoot, and fights hand
 * to hand once the enemy is close). Three ways to choose, and Manual: the original's ways for that member,
 * every order the player's. (1.4-RC4 had eight, RC5 to RC7 four with Man-at-arms: a Skirmisher without a
 * missile weapon does the same, so the maintainer dropped it.) */
#include <stdio.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "styles.h"

const int STYLE_ORDER[STYLE_CHOICES] = { ST_SKIRMISHER, ST_ARCHER, ST_CHARGER, ST_OFF };

static const char *NAMES[ST_N] = { "Manual", "Archer", "Skirmisher", "Skirmisher, hand to hand", "Charger" };
static const char *BRIEF[ST_N] = {
    "As in the original game: they wait for your orders.",
    "Shoots all fight, each archer at a different enemy; fights hand to hand only when reached, then shoots again.",
    "Shoots while the enemy approaches (if they have a missile weapon); once the enemy is close, fights hand to hand to the end of the fight.",
    "Fights hand to hand to the end of the fight.",
    "Goes straight for the enemy, their archers first.",
};
const char *STYLE_NOTE = "This only sets what they do when you haven't given an order. No bonus, no penalty: an order you give is carried out, then they go back to this.";
const char *style_name(int st) { return st >= 0 && st < ST_N ? NAMES[st] : ""; }
const char *style_brief(int st) { return st >= 0 && st < ST_N ? BRIEF[st] : ""; }

static void key_of(int m, const char *what, char *key, int n)
{
    char nm[26]; int k = 0;
    for (; k < 24; k++) { char ch = (char)g_rb((u16)(G_MEMBER(m) + G_M_FULLNAME + k)); if (!ch) break; nm[k] = ch == ' ' || ch == '=' ? '_' : ch; }
    nm[k] = 0;
    snprintf(key, n, "battle.%s.%s", what, nm);
}
static signed char test_style[5] = { -1, -1, -1, -1, -1 };   /* (ustyle:m,st, the test arena's: not kept in the settings) */
void style_test_set(int m, int st) { if (m >= 0 && m < 5) test_style[m] = (signed char)(st >= 0 && st <= ST_CHARGER && st != ST_FOOTMAN ? st : -1); }
int style_chosen(int m)
{
    if (m < 0 || m > 4) return -1;
    if (test_style[m] >= 0) return test_style[m];
    char key[48]; key_of(m, "style", key, sizeof key);
    int v = config_get_int(key, -1);
    static const signed char WAS[8] = { ST_OFF, ST_ARCHER, ST_SKIRMISHER, ST_SKIRMISHER, ST_CHARGER, ST_CHARGER, ST_SKIRMISHER, ST_ARCHER };   /* (RC5-RC7's Man-at-arms; RC4's Berserker, Bodyguard, Holdfast) */
    if (v >= 0 && v < 8) return WAS[v];
    return -1;
}
int style_suggested(int m) { (void)m; return ST_SKIRMISHER; }
int style_of(int m)
{
    int c = style_chosen(m);
    return c >= 0 ? c : style_suggested(m);
}
void style_set(int m, int st)
{
    if (m < 0 || m > 4) return;
    char key[48]; key_of(m, "style", key, sizeof key);
    config_set_int(key, st >= 0 && st <= ST_CHARGER && st != ST_FOOTMAN ? st : -1);
    config_save();
    fprintf(stderr, "styles: member %d fights as %s\n", m, st >= 0 ? style_name(st) : "suggested");
}
