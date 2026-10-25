/* enemyview.c - the battle's Enemy Info as a native page (display.native_enemyinfo; the view is
 * ui/enemyview.cpp).
 *
 * The original (enemy_info, s021 1ef8:02ce: E, or Orders > Enemy Info) draws at most two kinds of
 * foe into the side window at the left (each with its small picture from einfo.cat, the name of
 * its animation set, its armour, shield and weapons with a word for its skill) and waits for a
 * key or a click. With the party panel drawn by Darklands+ over that window, the words were
 * hidden and the battle seemed to stop. Here every kind on the field is shown (the combatants
 * 5..12; their records as a member's at G_MEMBER(i): +0x02 the kind, +0x0e the missile weapon,
 * +0x34 a weapon's magic, +0x37 / +0x38 body and leg armour, +0x3d the weapon, +0x3f natural
 * armour, +0x41 armour magic, +0x48 the shield, +0x57 / +0x5c the weapon skills), and it waits
 * the same way. */
#include <stdio.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"
void plat_hold(int ms);
#include "enemyview.h"

void plat_sleep_ms(int ms);
void dl_poll(void);

EnemyModel enemy_model;
static int ev_on = 1;

int enemyview_is_on(void) { return ev_on; }
/* combatant i (5-12) a man with armor and weapons, not a beast or a demon with natural ones: its pictures
   are a man's (the kind's picture prefix starts with E, as the enemy info screen tells them) */
int enemyview_human(int i)
{
    if (i < 5 || i >= 13) return 1;
    int kind = (s8)g_rb((u16)(G_MEMBER(i) + 2));
    if (kind < 0 || kind > 6) return 0;
    char pre[8]; g_str(G_DGROUP, g_rw((u16)(0x1166 + 2 * kind)), pre, sizeof pre);
    return pre[0] == 'E' || pre[0] == 'e';
}
void enemyview_set(int on) { ev_on = on; config_set_int("display.native_enemyinfo", on); }

static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

static void build(void)
{
    EnemyModel *m = &enemy_model;
    m->n = 0;
    m->difficulty = g_rb(GV_game_difficulty) & 0x7f;
    for (int q = 0; q < 7; q++) g_str(G_DGROUP, g_rw((u16)(GV_quality_words + 2 * q)), m->quality[q], sizeof m->quality[q]);
    for (int i = 5; i < 13; i++) {
        u16 b = G_MEMBER(i);
        int st = g_rb(b);
        if (!st) continue;
        int kind = (s8)g_rb((u16)(b + 2)), k;
        for (k = 0; k < m->n; k++) if (m->k[k].kind == kind) break;
        if (k == m->n) {
            if (m->n >= EV_KINDS || kind < 0 || kind > 6) continue;
            EnemyKind *e = &m->k[m->n++];
            memset(e, 0, sizeof *e);
            e->kind = kind; e->first = i;
            for (int c = 0; c < 0x14; c++) e->name[c] = (char)rb(G_FAR2E38, (u16)(0x0bd8 + kind * 0x14 + c));
            e->name[0x14] = 0;
            char pre[8], anim[12];
            g_str(G_DGROUP, g_rw((u16)(0x1166 + 2 * kind)), pre, sizeof pre);
            g_str(G_DGROUP, g_rw((u16)(GV_anim_file + 2 * kind)), anim, sizeof anim);
            char f[24]; snprintf(f, sizeof f, "%s%sST.PIC", pre, anim);
            for (char *c = f; *c; c++) if (*c >= 'a' && *c <= 'z') *c -= 0x20;
            snprintf(e->pic, sizeof e->pic, "EINFO.CAT|%s", f);
            e->human = f[0] == 'E';
            e->vital = (s8)g_rb((u16)(b + 0x37)); e->limb = (s8)g_rb((u16)(b + 0x38));
            e->shield = (s8)g_rb((u16)(b + 0x48));
            e->melee = (s8)g_rb((u16)(b + 0x3d)); e->missile = (s8)g_rb((u16)(b + 0x0e));
            e->armour_magic = (g_rb((u16)(b + 0x41)) & 0x7f) != 0;
            e->weapon_magic = g_rb((u16)(b + 0x34)) != 0;
            e->melee_skill = clampi((s8)g_rb((u16)(b + 0x57)), 0, 100) / 20;
            e->missile_skill = clampi((s8)g_rb((u16)(b + 0x5c)), 0, 100) / 20;
            e->natural = clampi(g_rb((u16)(b + 0x3f)) & 15, 0, 6);
            k = m->n - 1;
        }
        EnemyKind *e = &m->k[k];
        e->count++;
        if (st == 1) e->able++;
        if (e->nidx < 8) e->idx[e->nidx++] = i;
    }
    m->seq++;
}

static void info_hook(void)                 /* enemy_info (), s021 1ef8:02ce */
{
    if (!ev_on || !ns_active()) { hook_call_original(); return; }
    build();
    if (!enemy_model.n) { hook_call_original(); return; }
    plat_hold(600);                              /* (the old screen drawn while it is set up: not shown) */
    EnemyModel *m = &enemy_model;
    ns_unwinding = 0;
    m->open = 1;
    fprintf(stderr, "enemy info: open (%d kinds)\n", m->n);
    ns_enter(NS_ENEMY);
    ns_clear();
    ns_input_begin();
    for (;;) {                                   /* as the original: a key or a click */
        dl_poll();
        plat_sleep_ms(3);
        ns_input_poll();
        ns_call(GF_exit_if_requested, 0);
        if (ns_unwinding) break;
        if (ns_call(GF_key_pressed, 0)) break;
        if (ns_take_click(0, 0)) break;
    }
    if (!ns_unwinding) {
        while (g_rb(GV_mouse_buttons) & 1) { dl_poll(); plat_sleep_ms(3); }   /* (as the original: the button let go) */
        { void bios_kbd_drop_pending(void); bios_kbd_drop_pending(); }   /* (a key still on its way to the game: not given to it) */
        ns_call(GF_flush_keys, 0);
        ns_call(-1, 0x047c, 0x1320, 4, 6, 0, 0, 0);                         /* (the click taken: not one on the battle too) */
    }
    m->open = 0;
    ns_leave();
    if (ns_unwinding) return;
    ns_call(GF_party_panel_frames, 1, 0x100); ns_call(GF_party_panel_rows, 1, 0x100);
    fprintf(stderr, "enemy info: closed\n");
    hook_return_far(0);
}

void enemyview_init(void)
{
    ev_on = config_get_int("display.native_enemyinfo", 1);
    hook_install(GF_enemy_info, info_hook);
}
