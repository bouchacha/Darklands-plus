/* createview.c - making a character as a native screen (display.native_create; the view is
 * ui/createview.cpp).
 *
 * The original (create_character, s037 18c8:0004) is a screen of five states: the name and the
 * gender, the childhood, the attribute EPs, the profession, the skill EPs. Its left panel lists
 * what may be chosen now (rows clicked are keys 1..15 to its key handler), its right panel the
 * attributes and skills with what the profession under the pointer would bring. Here the same
 * handler (create_key, 18c8:058e) makes every step, with the attribute or skill row set as its
 * clicks set it (create_row) and + / - as its keys; the setup and the leaving are the original's.
 * What the view shows comes from the game's own tables: the professions (far 290e:2cc1, 0x3a
 * bytes: years, attributes, skills, the room each skill has), the childhoods (290e:2b65), the
 * aging after thirty (290e:3521), the profession names (290e:1ee7), the virtues and saints the
 * character comes to know (far 2e38:3b04, 3b1a). */
#include <stdio.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"
#include "createview.h"

u32 plat_ticks_ms(void);
void plat_sleep_ms(int ms);
void dl_poll(void);

CreateModel create_model;
static int cg_on = 1;
#define T290E ((u16)(LOADSEG + 0x290e))

int createview_is_on(void) { return cg_on; }
void createview_set(int on) { cg_on = on; config_set_int("display.native_create", on); }

static void far_str(u16 tseg, u16 toff, int i, char *out, int n)    /* entry i of a far table of far strings */
{
    u16 o = rw(tseg, (u16)(toff + 4 * i)), s = rw(tseg, (u16)(toff + 4 * i + 2));
    out[0] = 0;
    if (s) g_str(s, o, out, n);
}
static void prof_at(u16 off, CgProf *p)
{
    p->years = (s16)rw(T290E, off);
    for (int i = 0; i < 6; i++) p->attr[i] = (s8)rb(T290E, (u16)(off + 2 + i));
    for (int i = 0; i < 19; i++) { p->skill[i] = (s8)rb(T290E, (u16)(off + 8 + i)); p->maxsk[i] = (s8)rb(T290E, (u16)(off + 0x1b + i)); }
}

static void tables(void)                      /* what does not change while a character is made */
{
    CreateModel *c = &create_model;
    for (int i = 0; i < 37; i++) { prof_at((u16)(0x2cc1 + 0x3a * i), &c->adult[i]); far_str(T290E, 0x1ee7, i, c->prof_name[i], sizeof c->prof_name[i]); }
    for (int i = 0; i < 6; i++) { prof_at((u16)(0x2b65 + 0x3a * i), &c->childhood[i]); far_str(T290E, 0x20ab, i, c->child_name[i], sizeof c->child_name[i]); }
    for (int k = 0; k < 9; k++) for (int i = 0; i < 6; i++) c->aging[k][i] = (s8)rb(T290E, (u16)(0x3521 + 6 * k + i));
    for (int i = 0; i < 14; i++) far_str(T290E, 0x1df7, i, c->menu[i], sizeof c->menu[i]);
}

static void build(void)
{
    CreateModel *c = &create_model;
    c->state = (s16)g_rw(GV_create_state);
    g_str(G_DGROUP, GV_create_name, c->name, sizeof c->name);
    g_str(G_DGROUP, GV_create_nick, c->nick, sizeof c->nick);
    c->female = g_rb(GV_create_female) != 0;
    c->age = g_rw(GV_create_age);
    c->ep = (s16)g_rw(GV_create_ep); c->ep_step = (s16)g_rw(GV_create_ep_step);
    for (int i = 0; i < 7; i++) c->attr[i] = (s8)g_rb((u16)(GV_create_attr + i));
    for (int i = 0; i < 6; i++) c->attr_spent[i] = (s16)g_rw((u16)(GV_create_attr_spent + 2 * i));
    for (int i = 0; i < 19; i++) { c->skill[i] = (s8)g_rb((u16)(GV_create_skill + i)); c->skill_spent[i] = (s16)g_rw((u16)(GV_create_skill_spent + 2 * i)); }
    c->prof = (s16)g_rw(GV_create_prof); c->prev = (s16)g_rw(GV_create_prev); c->child = (s16)g_rw(GV_create_child);
    c->row = (s16)g_rw(GV_create_row);
    for (int i = 0; i < 37; i++) c->taken[i] = g_rb((u16)(GV_create_taken + i)) & 1;
    c->nlist = 0;
    if (c->state == 1) { for (int i = 0; i < 6; i++) c->list[c->nlist++] = i; }
    else if (c->state == 3) {
        int n = (s16)g_rw(GV_create_nlist);
        for (int i = 0; i < n && i < 15; i++) { int p = (s16)g_rw((u16)(GV_create_list + 2 * i)); if (p >= 0 && p < 37) c->list[c->nlist++] = p; }
    }
    /* the virtues (3 flags each) and the saints known so far */
    c->nvirtue = 0;
    for (int i = 0; i < 22; i++) {
        int f = rb(G_FAR2E38, (u16)(0x3b04 + i));
        for (int k = 0; k < 3 && c->nvirtue < 66; k++) if (f & (1 << k)) far_str(G_FAR2E38, (u16)(0x1736 + 12 * i), k, c->virtue[c->nvirtue++], 28);
    }
    c->nsaint = 0;
    for (int i = 0; i < 0x11; i++) for (int j = 0; j < 8; j++)
        if ((rb(G_FAR2E38, (u16)(0x3b1a + i)) & g_rb((u16)(0x00e8 + j))) && c->nsaint < 32) c->saint[c->nsaint++] = i * 8 + j;
    c->seq++;
}

static int key(int k) { return ns_call(GF_create_key, 1, k); }

static void set_text(u16 at, const char *t, int max)
{
    int i = 0;
    for (; t[i] && i < max; i++) g_wb((u16)(at + i), (u8)t[i]);
    g_wb((u16)(at + i), 0);
}

static void screen_hook(void)                 /* create_character (), s037 18c8:0004 */
{
    if (!cg_on || !ns_active()) { hook_call_original(); return; }
    CreateModel *c = &create_model;
    ns_unwinding = 0;
    /* the original's setup */
    g_ww(GV_create_state, 0);
    g_ww(GV_saved_back_font, g_rw(GV_back_win_font)); g_ww(GV_saved_screen_font, g_rw(GV_screen_win_font));
    ns_call(GF_create_reset, 0);
    ns_call(GF_create_load_pics, 0);
    ns_call(GF_create_draw, 0);
    if (ns_unwinding) return;
    tables();
    build();
    c->typing = 0;
    c->open = 1;
    fprintf(stderr, "creation: open\n");
    ns_enter(NS_CREATE);
    ns_clear();
    ns_input_begin();
    int done = 0;
    while (!done) {
        dl_poll();
        plat_sleep_ms(3);
        ns_input_poll();
        ns_call(GF_exit_if_requested, 0);
        if (ns_unwinding) break;
        while (ns_call(GF_key_pressed, 0)) {
            int k = (u16)ns_call(GF_get_key, 0);
            if (c->typing) {                                  /* the keys go into the name */
                int len = (int)strlen(c->typed), ch = k & 0xff, max = c->typing == 1 ? 22 : 9;
                if (k == 0x011b) c->typing = 0;
                else if (k == 0x1c0d) ns_push(CG_TYPED, 0, 0, 0);
                else if (k == 0x0e08) { if (len) c->typed[len - 1] = 0; }
                else if (ch >= 32 && ch < 127 && len < max) { c->typed[len] = (char)ch; c->typed[len + 1] = 0; }
                c->caret_t = plat_ticks_ms();
                continue;
            }
            if (k == 0x011b) { ns_push(CG_LEAVE, 0, 0, 0); continue; }
            if (k == 0x1000) { ns_call(GF_create_close, 0); ns_call(GF_quit_to_dos, 0); continue; }
            ns_push(CG_KEY, k, 0, 0);                         /* the game's own keys: B, K, N, M, S, + / - ... */
        }
        if (ns_unwinding) break;
        NsAction a;
        while (!done && !ns_unwinding && ns_pop(&a)) {
            switch (a.type) {
            case CG_KEY: if (key(a.a) == 0x11b) done = 1; break;
            case CG_ROWPLUS: case CG_ROWMINUS:
                g_ww(GV_create_row, (u16)a.a);
                key(a.type == CG_ROWPLUS ? 0x0d2b : 0x0c2d);
                break;
            case CG_TYPE:
                c->typing = a.a;
                snprintf(c->typed, sizeof c->typed, "%s", a.a == 1 ? c->name : c->nick);
                c->caret_t = plat_ticks_ms();
                break;
            case CG_TYPED:
                if (c->typing == 1 && c->typed[0]) { set_text(GV_create_name, c->typed, 22); g_ww(GV_create_random_name, 0); }
                if (c->typing == 2 && c->typed[0]) { set_text(GV_create_nick, c->typed, 9); g_ww(GV_create_random_name, 0); }
                c->typing = 0;
                break;
            case CG_LEAVE: {                                  /* Esc, as the original: back a step, or out */
                int st = (s16)g_rw(GV_create_state);
                if (st == 2 || st == 4) { g_ww(GV_create_state, (u16)(st - 1)); ns_call(GF_create_draw, 0); }
                else done = 1;
                break;
            }
            }
            if (!ns_unwinding) build();
        }
        if (ns_unwinding) break;
    }
    c->open = 0;
    ns_leave();
    fprintf(stderr, "creation: closed\n");
    if (ns_unwinding) return;
    ns_call(GF_create_close, 0);
    hook_return_far(0);
}

void createview_init(void)
{
    cg_on = config_get_int("display.native_create", 1);
    hook_install(GF_create_character, screen_hook);
}

void createview_dump(void)                    /* (the test command ucg) */
{
    CreateModel *c = &create_model;
    tables(); build();
    for (int i = 0; i < 37; i++) fprintf(stderr, "prof %d %s years %d\n", i, c->prof_name[i], c->adult[i].years);
    for (int i = 0; i < 6; i++) fprintf(stderr, "child %d %s\n", i, c->child_name[i]);
    fprintf(stderr, "state %d age %d prof %d prev %d child %d list:", c->state, c->age, c->prof, c->prev, c->child);
    for (int i = 0; i < c->nlist; i++) fprintf(stderr, " %d", c->list[i]);
    int cg_offered_c(int *out); int o[40], n = cg_offered_c(o);
    fprintf(stderr, "\nrules:");
    for (int i = 0; i < n; i++) fprintf(stderr, " %d", o[i]);
    fprintf(stderr, "\n");
}
