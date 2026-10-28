/* styles.c - each party member's way of fighting: what they do in a battle while the player gives them no
 * order (Improved combat, mods/orders.c carries it out; an order of the player's is theirs for the rest of
 * that fight, in its manner: sent hand to hand, they go on hand to hand; sent to shoot, they go on shooting;
 * sent somewhere or halted, they hold there). Chosen on the member's box in the party panel (ui/partypanel.cpp)
 * and kept by the character's name (battle.style.<name>), so it holds from battle to battle and save to save.
 * Until one is chosen: Skirmisher for a member with a missile weapon in hand, Man-at-arms for the others, which
 * is the way most players fight (shoot while the enemies come, everyone on a foe of their own when they are
 * close). Off: the original's ways for that member. */
#include <stdio.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "styles.h"

const int STYLE_ORDER[ST_N] = { ST_SKIRMISHER, ST_ARCHER, ST_HOLDFAST, ST_FOOTMAN, ST_CHARGER, ST_BERSERKER, ST_BODYGUARD, ST_OFF };

static const char *NAMES[ST_N] = { "Off", "Archer", "Skirmisher", "Man-at-arms", "Charger", "Berserker", "Bodyguard", "Holdfast" };
static const char *BRIEF[ST_N] = {
    "No orders of its own: the original game's ways. You give every order.",
    "Shoots from the start and all fight long, each archer at a foe of its own, the enemy's shooters first; fights only a foe that reaches it, then takes up the bow again.",
    "Shoots while the enemies come; once one is close, draws the melee weapon for the rest of the fight and fights as a Man-at-arms.",
    "Stays with the party while the others shoot; when the enemies are close, joins a friend's fight (two on one) or takes a foe no one fights yet, until the end.",
    "Goes at the enemies from the start: their shooters first, then the nearest one no one of the party fights yet.",
    "As the Charger, with Berserk blows (quick, furious, unguarded); Vulnerable where only that gets through the armor; Standard once badly hurt.",
    "Keeps beside the most exposed of the party (a shooter, or the one in the lightest armor) and fights whoever comes at them; parries when outnumbered.",
    "Never moves on its own: shoots what it can see from where it stands, fights only a foe that reaches it, and parries when set upon by several.",
};
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
void style_test_set(int m, int st) { if (m >= 0 && m < 5) test_style[m] = (signed char)(st >= 0 && st < ST_N ? st : -1); }
int style_chosen(int m)
{
    if (m < 0 || m > 4) return -1;
    if (test_style[m] >= 0) return test_style[m];
    char key[48]; key_of(m, "style", key, sizeof key);
    int v = config_get_int(key, -1);
    if (v >= 0 && v < ST_N) return v;
    key_of(m, "shoot", key, sizeof key);                  /* (1.3's switch on the box: the sword chosen) */
    if (config_get_int(key, -1) == 0) return ST_FOOTMAN;
    return -1;
}
int style_suggested(int m)
{
    if (m < 0 || m > 4) return ST_FOOTMAN;
    int wm = (s8)g_rb(CBF(m, CB_MISSILE));
    return wm >= 0 && wm <= 62 ? ST_SKIRMISHER : ST_FOOTMAN;
}
int style_of(int m)
{
    int c = style_chosen(m);
    return c >= 0 ? c : style_suggested(m);
}
void style_set(int m, int st)
{
    if (m < 0 || m > 4) return;
    char key[48]; key_of(m, "style", key, sizeof key);
    config_set_int(key, st >= 0 && st < ST_N ? st : -1);
    config_save();
    fprintf(stderr, "styles: member %d fights as %s\n", m, st >= 0 ? style_name(st) : "suggested");
}
