/* foefx.c - what a thrown potion (or a trap's) is doing to each fighter now, and for how long: the
 * battle's own records, read for ui/battleoverlay.cpp (a picture over the fighter, the words under the
 * pointer). His ask (1.4.7-RC6): "I have no idea how long fleadust lasts".
 *
 * effect_spell (s019 1ef8:0000, x, y, p3, type, dur, target, side; src/c/s019) puts a potion's cloud,
 * flash or pool on the field; type is the potion less 95 (0 Noxious Aroma ... 9 Stone-tar). What it does
 * to a fighter i is kept in cbt_mods[i] (DS:a532, 0x12 bytes): +0, +1 how much Agility and Perception
 * were lowered, +2..+8 the seven weapon skills, +9 held (blinded: the fighter only parries, a shot or a
 * throw stopped), +0c, +0e, +10 when each of the three ends on the battle's clock (DS:07d4, about 60
 * ticks a second), -536 none. Fleadust lowers the skills alone (5 x the armor worn on body and limbs,
 * in hundredths); Noxious Aroma and Eyeburn the attributes and the skills, Eyeburn and Sunburst blind.
 * The game keeps no record of which potion it was: effect_spell is watched here, the ends it changes
 * noted with the potion and when; one not seen (a battle loaded) is named from what it does. */
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "nscreen.h"
#include "foefx.h"

#define MOD(i) ((u16)(GV_cbt_mods + 0x12 * (i)))
#define NONE ((s16)-536)

static struct { s16 end[3]; int potion[3]; u16 start[3]; } seen[13];   /* (per end: +0e skills, +0c attributes, +10 held) */
static const u16 END_AT[3] = { 0x0e, 0x0c, 0x10 };

static void spell_hook(void)                       /* effect_spell (x, y, p3, type, dur, target, side) */
{
    int type = (s16)rw(SS, (u16)(SP + 10));
    s16 before[13][3];
    for (int i = 0; i < 13; i++) for (int k = 0; k < 3; k++) before[i][k] = (s16)g_rw((u16)(MOD(i) + END_AT[k]));
    hook_call_original();
    if (ns_unwinding) return;
    u16 now = g_rw(GV_combat_clock);
    for (int i = 0; i < 13; i++) for (int k = 0; k < 3; k++) {
        s16 e = (s16)g_rw((u16)(MOD(i) + END_AT[k]));
        if (e == before[i][k] || e == NONE) continue;
        seen[i].end[k] = e; seen[i].potion[k] = type >= 0 && type <= 9 ? 95 + type : -1; seen[i].start[k] = now;
    }
}

static int pct_of(int lowered, int now) { return lowered > 0 && lowered + now > 0 ? (lowered * 100 + (lowered + now) / 2) / (lowered + now) : 0; }

int foefx_of(int i, FoeFx *out, int max)
{
    if (i < 0 || i >= 13 || max <= 0) return 0;
    u16 now = g_rw(GV_combat_clock), c = (u16)(GV_combatants + 0x80 * i);
    if (g_rb((u16)(c + CB_STATUS)) != 1) return 0;
    int left[3];
    for (int k = 0; k < 3; k++) {
        s16 e = (s16)g_rw((u16)(MOD(i) + END_AT[k]));
        left[k] = e == NONE ? 0 : (s16)(e - (s16)now);
        if (left[k] < 0) left[k] = 0;
    }
    if (!g_rb((u16)(MOD(i) + 9))) left[2] = 0;                 /* (held: only while +9 says so) */
    int skills = 0;                                            /* the weapon skill lowered most, in hundredths */
    for (int k = 0; k < 7; k++) { int p = pct_of(g_rb((u16)(MOD(i) + 2 + k)), g_rb((u16)(c + 0x6b + k))); if (p > skills) skills = p; }
    int attrs = pct_of(g_rb(MOD(i)), g_rb((u16)(c + 0x5f)));
    int n = 0;
    for (int k = 0; k < 3 && n < max; k++) {
        if (!left[k] || (k == 0 && left[1])) continue;         /* (skills and attributes lowered together: one, Weakened) */
        FoeFx f; memset(&f, 0, sizeof f);
        f.what = k == 0 ? FX_SKILLS : k == 1 ? FX_WEAKENED : FX_BLIND;
        f.left = left[k];
        f.pct = k == 0 ? skills : k == 1 ? (attrs > skills ? attrs : skills) : 0;
        s16 e = (s16)g_rw((u16)(MOD(i) + END_AT[k]));
        if (seen[i].end[k] == e && seen[i].potion[k] >= 0) { f.potion = seen[i].potion[k]; f.total = (s16)(e - (s16)seen[i].start[k]); }
        else f.potion = k == 0 ? 98 : k == 2 ? (left[1] ? 96 : 101) : left[2] ? 96 : -1;   /* (Fleadust lowers the skills alone; blinded and weakened: Eyeburn) */
        out[n++] = f;
    }
    /* one entry a potion (his ask, 1.4.7: "effect from same potion shows up twice"): Eyeburn's blinding with what it
       lowers, the dial the longer of the two */
    for (int b = 0; b < n; b++) {
        if (out[b].what != FX_BLIND || out[b].potion < 0) continue;
        for (int o = 0; o < n; o++) {
            if (o == b || out[o].what == FX_BLIND || out[o].potion != out[b].potion) continue;
            out[o].blind = 1; out[o].blind_left = out[b].left;
            if (out[b].left > out[o].left) { out[o].left = out[b].left; out[o].total = out[b].total; }
            for (int j = b; j + 1 < n; j++) out[j] = out[j + 1];
            n--; b--;
            break;
        }
    }
    return n;
}

void foefx_init(void)
{
    hook_install(19, 0x1ef8, 0x0000, spell_hook);
}
