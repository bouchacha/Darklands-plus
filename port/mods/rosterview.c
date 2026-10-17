/* rosterview.c - the party of a new game as a native screen (display.native_roster; the view is
 * ui/rosterview.cpp).
 *
 * The original (roster_screen, s037 1462:0000, pics\cretscrn.pic) is a menu of words on the left
 * (Create, Add, Heraldry, Image, the three colours, Drop, Kill, Begin, Return), the characters
 * kept in charactr.tmp on the right, the party's boxes beside them and the chosen member's
 * portrait below. Here its key handler (roster_key, 1462:0444) still does every one of those with
 * the character chosen as its arrow keys choose (roster_sel); the setup and the leaving are the
 * original's. What the view shows is read from charactr.tmp itself (int count, int ids[5] party
 * slot -> character, char figure[5][4], then 0x22a bytes a character: the 0x80-byte record as a
 * party member's, the saints known, the formulae known, the things carried) and the slots'
 * colours and heralds from the game's memory. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"
#include "rosterview.h"
#include "looks.h"

u32 plat_ticks_ms(void);
void plat_sleep_ms(int ms);
void dl_poll(void);
void dos_host_path_read(const char *dos, char *out, size_t outn);

RosterModel roster_model;
static int rv_on = 1;
#define VECSEG ((u16)(LOADSEG + 0x9c0))

int rosterview_is_on(void) { return rv_on; }
void rosterview_set(int on) { rv_on = on; config_set_int("display.native_roster", on); }

static int bits(const unsigned char *p, int n) { int c = 0; for (int i = 0; i < n; i++) for (int b = p[i]; b; b &= b - 1) c++; return c; }
static void cstr(char *out, int n, const unsigned char *p, int max) { int i = 0; for (; i < max && i < n - 1 && p[i]; i++) out[i] = (char)p[i]; out[i] = 0; }

static void build(void)
{
    RosterModel *r = &roster_model;
    char host[1200];
    r->n = 0;
    for (int i = 0; i < 5; i++) r->slot_char[i] = -1;
    dos_host_path_read("charactr.tmp", host, sizeof host);
    FILE *f = fopen(host, "rb");
    if (f) {
        unsigned char h[32], rec[0x22a];
        if (fread(h, 1, 32, f) == 32) {
            int n = (s16)(h[0] | h[1] << 8);
            for (int i = 0; i < 5; i++) r->slot_char[i] = (s16)(h[2 + 2 * i] | h[3 + 2 * i] << 8);
            for (int i = 0; i < n && r->n < RV_MAX && fread(rec, 1, sizeof rec, f) == sizeof rec; i++) {
                RosterChar *c = &r->ch[r->n++];
                memset(c, 0, sizeof *c);
                cstr(c->name, sizeof c->name, rec + 0x25, 25);
                cstr(c->nick, sizeof c->nick, rec + 0x3e, 11);
                c->age = (s16)(rec[0x12] | rec[0x13] << 8);
                c->female = rec[0x17] != 0;
                for (int a = 0; a < 7; a++) { c->attr[a] = (s8)rec[0x5d + a]; c->attr_max[a] = (s8)rec[0x64 + a]; }
                for (int s = 0; s < 19; s++) c->skill[s] = (s8)rec[0x6b + s];
                c->nitems = rec[0x7e];
                c->nsaints = bits(rec + 0x80, 0x14);
                c->nformulae = bits(rec + 0x94, 0x16);
                c->slot = -1;
            }
        }
        fclose(f);
    }
    r->party_n = 0;
    for (int s = 0; s < 5; s++) {
        int c = r->slot_char[s];
        if (c >= 0 && c < r->n) { r->ch[c].slot = s; r->party_n++; }
        else r->slot_char[s] = -1;
        for (int k = 0; k < 4; k++) r->figure[s][k] = (char)g_rb((u16)(GV_member_shield + 4 * s + k));
        r->figure[s][3] = 0;
        for (int k = 0; k < 24; k++) r->pal[s][k] = rb(G_FAR20A5, (u16)(0x33fc + 24 * s + k)) & 63;
        r->herald[s] = (char)g_rb((u16)(G_MEMBER(s) + 1));
    }
    r->sel = (s16)g_rw(GV_roster_sel);
    if (r->sel >= r->n) r->sel = r->n - 1;
    r->seq++;
}

static void words(void)                       /* the original's menu (1462:2922): F_290e_0000.s[] */
{
    static const int IDX[11] = { 1, 0, 0x40, 0x1a, 0x3d, 0x3e, 0x3f, 3, 0x13, 4, 5 };
    u16 seg = (u16)(LOADSEG + 0x290e);
    for (int i = 0; i < 11; i++) {
        u16 o = rw(seg, (u16)(0x1ddf + 4 * IDX[i])), s = rw(seg, (u16)(0x1ddf + 4 * IDX[i] + 2));
        roster_model.word[i][0] = 0;
        if (s) g_str(s, o, roster_model.word[i], sizeof roster_model.word[i]);
        if (getenv("DL_ROSTER_LOG")) fprintf(stderr, "roster: word %d '%s'\n", i, roster_model.word[i]);
    }
}

static void note(const char *t) { snprintf(roster_model.note, sizeof roster_model.note, "%s", t); roster_model.note_t = plat_ticks_ms(); }
static int key(int k) { return ns_call(GF_roster_key, 1, k); }

static void choose(int i)                     /* as its arrow keys do (the portrait, its colours) */
{
    if (i < 0 || i >= roster_model.n) return;
    /* from a neighbour (its list has room for 20 rows: never from beyond the last) */
    if (i > 0) { g_ww(GV_roster_sel, (u16)(i - 1)); key(0x5000); }
    else { g_ww(GV_roster_sel, (u16)(roster_model.n > 1 ? 1 : 0)); key(0x4800); }
}

static void redraw(int slot)                  /* the screen again after a sheet (as the original) */
{
    u16 sp = SP;
    ns_call(GF_screen_hold, 0);
    u16 name = ns_stack_str("pics\\cretscrn.pic");
    if (SS == G_DGROUP) ns_call(GF_pic_draw_pal, 3, 1, name, GV_cur_palette);
    ns_stack_free(sp);
    for (int k = 0; k < 24; k++) g_wb((u16)(0xe753 + k), rb(G_FAR20A5, (u16)(0x33fc + 24 * slot + k)));
    ns_call(GF_set_palette, 1, GV_cur_palette);
    ns_call(GF_roster_draw, 0);
    ns_call(GF_screen_release, 0);
}

static void screen_hook(void)                 /* roster_screen (), s037 1462:0000 */
{
    if (!rv_on || !ns_active()) { hook_call_original(); return; }
    RosterModel *r = &roster_model;
    ns_unwinding = 0;
    /* the original's setup */
    ns_call_ptr(VECSEG, 0x18f1, 1, 0x14);
    g_ww(GV_saved_back_font, g_rw(GV_back_win_font)); g_ww(GV_saved_screen_font, g_rw(GV_screen_win_font));
    ns_call(GF_screen_hold, 0);
    ns_call(GF_roster_figures, 0);
    g_ww(GV_roster_portrait, 0);
    {
        u16 sp = SP, name = ns_stack_str("default");
        ns_call(GF_load_saved_game, 2, name, SS);
        ns_stack_free(sp);
    }
    for (int k = 0; k < 5; k++) g_wb(G_MEMBER(k), 0);
    ns_call(GF_roster_load_pics, 0);
    ns_call(GF_roster_draw, 0);
    ns_call(GF_screen_release, 0);
    if (ns_unwinding) return;
    words();
    build();
    r->note[0] = 0;
    r->open = 1;
    fprintf(stderr, "roster: open (%d characters, %d in the party)\n", r->n, r->party_n);
    ns_enter(NS_ROSTER);
    ns_clear();
    ns_input_begin();
    int result = -1;
    while (result < 0) {
        dl_poll();
        plat_sleep_ms(3);
        ns_input_poll();
        ns_call(GF_exit_if_requested, 0);
        if (ns_unwinding) break;
        while (ns_call(GF_key_pressed, 0)) {
            int k = (u16)ns_call(GF_get_key, 0);
            if (k == 0x011b || k == 0x1352 || k == 0x1372) ns_push(RV_BACK, 0, 0, 0);
            else if (k == 0x1000) { ns_call(GF_quit_to_dos, 0); }
            else if (k == 0x4800 || k == 0x4b00) ns_push(RV_SELECT, r->sel > 0 ? r->sel - 1 : r->n - 1, 0, 0);
            else if (k == 0x5000 || k == 0x4d00) ns_push(RV_SELECT, r->sel + 1 < r->n ? r->sel + 1 : 0, 0, 0);
            else ns_push(RV_KEY, k, 0, 0);
        }
        if (ns_unwinding) break;
        NsAction a;
        while (result < 0 && !ns_unwinding && ns_pop(&a)) {
            int before = r->party_n, nbefore = r->n;
            switch (a.type) {
            case RV_SELECT: choose(a.a); break;
            case RV_KEY: {
                int lo = a.a & 0xdf;                              /* the letter, either case */
                if (lo == 'B' && !r->party_n) { note("Add someone to the party first."); break; }
                if (lo == 'A' && r->sel >= 0 && r->ch[r->sel].slot < 0 && r->party_n >= 4) { note("The party is full: four travel together. Drop someone first."); break; }
                if (key(a.a) == 0x11b) result = 1;
                break;
            }
            case RV_SHEET:
                if (a.a >= 0 && a.a < 5 && g_rb(G_MEMBER(a.a))) {
                    ns_call_ptr(VECSEG, 0x1fdb, 1, a.a);
                    if (!ns_unwinding) redraw(a.a);
                }
                break;
            case RV_BACK: result = 0; break;
            }
            if (ns_unwinding) break;
            build();
            if (a.type == RV_KEY && (a.a & 0xdf) == 'C' && r->n > nbefore) { choose(r->n - 1); build(); note("The new character is kept: add them to the party."); }
            if (a.type == RV_KEY && (a.a & 0xdf) == 'A' && r->party_n > before) {
                note("");
                /* a woman gets the game's woman figure (it gives everyone the first, the tall man): Select Image with it */
                const RosterChar *c = r->sel >= 0 && r->sel < r->n ? &r->ch[r->sel] : 0;
                if (c && c->female && c->slot >= 0 && !strncmp(r->figure[c->slot], "F01", 3) && config_get_int("party.woman_figure", 1)) looks_pick("F60");
            }
        }
        if (ns_unwinding) break;
    }
    r->open = 0;
    ns_leave();
    fprintf(stderr, "roster: closed (%s)\n", result == 1 ? "begin" : ns_unwinding ? "unwound" : "back");
    if (ns_unwinding) return;
    ns_call(GF_roster_leave, 1, result);
    AX = (u16)result;
    hook_return_far(0);
}

void rosterview_init(void)
{
    rv_on = config_get_int("display.native_roster", 1);
    hook_install(GF_roster_screen, screen_hook);
}
