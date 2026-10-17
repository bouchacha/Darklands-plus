/* memberlooks.c - characters keep their figure and colours when they rejoin the party
 * (party.keep_looks), and newcomers at an inn get a look of their own (party.new_member_looks).
 *
 * The figure (DS:a668, 4 bytes a party slot) and the colours (far 20a5:33fc, 24 bytes a slot)
 * belong to the party slot, not to the character: the inn's party screen (s038 1462:0000; its
 * key handler 02f8, A: add or drop) gives whoever joins the first figure (the tall man, F01)
 * in fixed colours. So a member who retired and joins again, or a character made at the inn,
 * looks like everyone else added. Here the looks of the members are remembered by name (in
 * the settings, look.<name> = figure + colours) whenever they are in the party outside the
 * party screens, and given back after A; a character without a remembered look gets the woman
 * figure for a woman and the next colours of the 1st and 2nd colour keys, as at a new game. */
#include <stdio.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../runtime/dispatch.h"
#include "nscreen.h"
#include "looks.h"

#define G_FAR20A5 ((u16)(LOADSEG + 0x20a5))
#define PAL(m) ((u16)(0x33fc + (m) * 24))
#define FIG(m) ((u16)(GV_member_shield + (m) * 4))

static int on(void) { return config_get_int("party.keep_looks", 1) && !config_get_int("original", 0); }

static void name_of(int m, char *out, int n)
{
    int k = 0;
    for (; k < n - 1 && k < 24; k++) {
        char c = (char)g_rb((u16)(G_MEMBER(m) + G_M_FULLNAME + k));
        if (!c) break;
        out[k] = (c == ' ' || c == '=') ? '_' : c;
    }
    out[k] = 0;
}
static void hex(const unsigned char *b, int n, char *out) { for (int i = 0; i < n; i++) sprintf(out + 2 * i, "%02x", b[i]); }
static int unhex(const char *s, unsigned char *b, int n)
{
    for (int i = 0; i < n; i++) { unsigned v; if (sscanf(s + 2 * i, "%2x", &v) != 1) return 0; b[i] = (unsigned char)v; }
    return 1;
}
static void read_look(int m, unsigned char look[28])
{
    for (int i = 0; i < 4; i++) look[i] = g_rb((u16)(FIG(m) + i));
    for (int i = 0; i < 24; i++) look[4 + i] = rb(G_FAR20A5, (u16)(PAL(m) + i));
}
static void write_look(int m, const unsigned char look[28])
{
    char code[4] = { (char)look[0], (char)look[1], (char)look[2], 0 };
    looks_set_figure(m, code);                                  /* (in charactr.tmp too: the battles take it from there) */
    for (int i = 0; i < 24; i++) wb(G_FAR20A5, (u16)(PAL(m) + i), look[4 + i]);
}

/* every frame, outside the party screens and creation: the members' looks remembered */
void memberlooks_frame(void)
{
    static unsigned n; if (++n % 30) return;                 /* (twice a second is plenty) */
    if (!on() || game_mode() == GM_OTHER || ovl_loaded[37] || ovl_loaded[38]) return;
    int size = (s16)g_rw(G_PARTY_SIZE);
    for (int m = 0; m < size && m < 5; m++) {
        if (!g_rb(G_MEMBER(m))) continue;
        unsigned char look[28]; read_look(m, look);
        if (look[0] < 'A' || look[0] > 'Z') continue;
        char nm[32], key[48], val[64]; name_of(m, nm, sizeof nm);
        if (!nm[0]) continue;
        snprintf(key, sizeof key, "look.%s", nm); hex(look, 28, val);
        if (strcmp(config_get(key, ""), val)) { config_set(key, val); config_save(); fprintf(stderr, "looks: %s remembered\n", nm); }
    }
}

/* the inn's party screen: A (add or drop) */
static void key_hook(void)                  /* s038 1462:02f8 (key) */
{
    int k = (u16)rw(SS, (u16)(SP + 4));
    if (!on() || (k != 0x1e41 && k != 0x1e61)) { hook_call_original(); return; }
    int had[5]; for (int m = 0; m < 5; m++) had[m] = g_rb(G_MEMBER(m)) != 0;
    hook_call_original();
    u16 ax = AX, dx = DX;
    for (int m = 0; m < 5; m++) {
        if (had[m] || !g_rb(G_MEMBER(m))) continue;          /* (only who just joined) */
        char nm[32], key[48]; name_of(m, nm, sizeof nm);
        snprintf(key, sizeof key, "look.%s", nm);
        unsigned char look[28];
        const char *v = config_get(key, "");
        if (strlen(v) == 56 && unhex(v, look, 28) && look[0] >= 'A' && look[0] <= 'Z') {
            write_look(m, look);
            fprintf(stderr, "looks: %s joins again with %.3s and their colours\n", nm, (const char *)look);
        } else if (config_get_int("party.new_member_looks", 1)) {
            /* a newcomer: as at a new game (rosterview.c) - a woman the woman figure, the next colours */
            int female = g_rb((u16)(G_MEMBER(m) + 3)) != 0;
            if (female) looks_set_figure(m, "F60");
            u16 a1[1] = { 0x0231 }, a2[1] = { 0x0332 };
            int game_call_far(int sect, u16 seg, u16 off, int nargs, const u16 *args);
            game_call_far(38, 0x1462, 0x02f8, 1, a1);
            game_call_far(38, 0x1462, 0x02f8, 1, a2);
            fprintf(stderr, "looks: %s joins with a look of their own%s\n", nm, female ? " (the woman figure)" : "");
        }
        g_ww(GV_party_icons_dirty, 1);
    }
    AX = ax; DX = dx;
}

void memberlooks_init(void)
{
    hook_install(38, 0x1462, 0x02f8, key_hook);
}
