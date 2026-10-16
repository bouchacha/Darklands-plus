/* sheetview.c - a member's sheet as a native screen (nscreen.h; the view is ui/sheetview.cpp).
 *
 * The original (member_sheet, s031 1462:0124) shows the portrait and boxes; the pack is a
 * 14-line box opened by clicking Equipment, worked with keys (A ready, U put away, 1..5 give,
 * D drop, P drink). Here the setup and the leaving are the original's own (09b8 / 08f6: fonts,
 * cursor, buffers), and the actions are the routines its key handler calls:
 *   ready      s031 18e7:11b2 (who, idx)          put away  18e7:12e8 item_unequip (who, idx, 9)
 *   give one   18e7:0000 (from, to, idx, 9)       give all  18e7:007c give_stack
 *   drop one   18e7:0474 item_take_one (who, idx) drop all  18e7:07d4 item_remove_stack
 *   drink      1462:43b2 (who, idx), then item_take_one (as the original's P)
 * In a battle the same, but a member on another level of the battlefield cannot be shown (the
 * original's message instead, as it does). */
#include <stdio.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "nscreen.h"
#include "gamemodel.h"
#include "sheetview.h"

SheetModel sheet_model;
static int sv_on = 1;
u32 plat_ticks_ms(void);

int sheetview_is_on(void) { return sv_on; }
void sheetview_set(int on) { sv_on = on; config_set_int("display.native_sheet", on); }


static void note(const char *t) { snprintf(sheet_model.note, sizeof sheet_model.note, "%s", t); sheet_model.note_t = plat_ticks_ms(); fprintf(stderr, "sheet: %s\n", t); }

static int drinkable(int id)
{
    unsigned long c;
    return gm_itemdef_cats(id, &c) && (c & 0x800) && !(c & 0x1000000);
}

static void build(int who)
{
    SheetModel *s = &sheet_model;
    s->who = who;
    s->leader = gm_leader();
    s->npar = gm_party_size();
    for (int m = 0; m < 5; m++) {
        s->present[m] = gm_member_present(m);
        s->able[m] = gm_member_status(m) == 1;
        gm_member_name(m, s->names[m], sizeof s->names[m]);
    }
    gm_member_fullname(who, s->fullname, sizeof s->fullname);
    s->female = gm_member_female(who);
    u16 b = G_MEMBER(who);
    s->age = (s16)g_rw((u16)(b - 2));
    for (int a = 0; a < 7; a++) { s->attr[a] = (s8)g_rb((u16)(b + G_M_ATTR_CUR + a)); s->attr_max[a] = (s8)g_rb((u16)(b + G_M_ATTR_MAX + a)); }
    for (int k = 0; k < 19; k++) s->skill[k] = (s8)g_rb((u16)(b + G_M_SKILLS + k));
    s->load = g_rb((u16)(b + G_M_LOAD));
    s->capacity = s->attr[0] + s->attr[1];
    s->nitems = gm_item_count(who);
    for (int i = 0; i < s->nitems; i++) {
        GmItem g; gm_item(who, i, &g);
        SvItem *it = &s->item[i];
        it->idx = i; it->id = g.id; it->type = g.type; it->qual = g.qual; it->count = g.count; it->weight = g.weight;
        it->equipped = gm_item_equipped(who, i);
        it->drinkable = drinkable(g.id);
    }
    for (int k = 0; k < 66; k++) s->formula[k] = (unsigned char)gm_knows_formula(who, k);
    for (int k = 0; k < 160; k++) s->saint[k] = (unsigned char)gm_knows_saint(who, k);
    s->seq++;
}

static void item_name(int id, char *out, int n) { gm_itemdef_name(id, out, n); }

/* the paper doll (the original's left half, kept on show): drawn again by the original's own
 * 0b22 into the back window and copied to the screen as its key handler does */
/* one of the original sheet's boxes drawn again: 39a0(box, who, screen_win), the window passed by
 * value (0x12 bytes); box 0 the name bar (Leader / Not Leader), 2 the load */
static void box_redraw(int box, int who)
{
    u16 w[9]; for (int i = 0; i < 9; i++) w[i] = g_rw((u16)(GV_screen_win + 2 * i));
    ns_call(GF_sheet_box_draw, 11, box, who, w[0], w[1], w[2], w[3], w[4], w[5], w[6], w[7], w[8]);
}
static void doll_redraw(int who)
{
    ns_call(GF_sheet_doll, 1, who);
    ns_call(GF_gfx_copy_rect, 8, GV_back_win, 60, 13, 138, 170, GV_screen_win, 60, 13);
    box_redraw(2, who);
}
/* another member: the whole original sheet drawn for them (their doll, frame, load) */
static void member_redraw(int who)
{
    ns_call(GF_party_panel_frames, 1, 0x100); ns_call(GF_party_panel_rows, 1, 0x100);
    ns_call(GF_sheet_portrait_frame, 2, who, 11);
    ns_call(GF_sheet_member, 1, who);
}

/* the state an action may change, to tell whether it did anything */
typedef struct { int n, cnt, eq, load; } Snap;
static Snap snap(int who, int idx)
{
    Snap s = { gm_item_count(who), 0, 0, g_rb((u16)(G_MEMBER(who) + G_M_LOAD)) };
    GmItem g; if (gm_item(who, idx, &g)) { s.cnt = g.count; s.eq = gm_item_equipped(who, idx); }
    return s;
}
static int changed(Snap a, Snap b) { return a.n != b.n || a.cnt != b.cnt || a.eq != b.eq || a.load != b.load; }

static void act(const NsAction *a, int *who)
{
    int w = *who, idx = a->a;
    GmItem g; char nm[32] = "", t[160];
    if (a->type != SV_WHO && a->type != SV_LEADER && a->type != SV_CLOSE) {
        if (!gm_item(w, idx, &g)) return;
        item_name(g.id, nm, sizeof nm);
    }
    Snap s0 = snap(w, idx);
    switch (a->type) {
    case SV_WHO:
        if (g_rb(GV_in_battle) == 1 && (u8)g_rb((u16)(GV_cbt_floor + a->a)) == 0x9d) { ns_call(GF_member_away_msg, 1, a->a); return; }
        if (gm_member_present(a->a) && gm_member_status(a->a) == 1) {
            *who = a->a; g_wb(GV_sheet_who, (u8)a->a);
            sheet_model.note[0] = 0;
            member_redraw(*who);
        }
        return;
    case SV_LEADER:
        if (gm_member_present(a->a) && gm_member_status(a->a) == 1) { g_wb(G_LEADER, (u8)a->a); ns_call(GF_party_panel_rows, 1, 0x100); box_redraw(0, w); }
        return;
    case SV_READY:
        ns_call(GF_item_ready, 2, w, idx);
        if (changed(s0, snap(w, idx))) snprintf(t, sizeof t, "%s takes up the %s.", sheet_model.names[w], nm);
        else snprintf(t, sizeof t, "The %s cannot be readied.", nm);
        break;
    case SV_PUTAWAY:
        ns_call(GF_item_unequip, 3, w, idx, 9);
        snprintf(t, sizeof t, "%s puts the %s away.", sheet_model.names[w], nm);
        break;
    case SV_GIVE: {
        int to = a->b, all = a->c == 255;
        if (to == w || !gm_member_present(to) || gm_member_status(to) == 5) return;
        int before = gm_item_count(to);
        if (before >= 64 && !all) { snprintf(t, sizeof t, "%s cannot carry another kind of thing.", sheet_model.names[to]); break; }
        if (all) ns_call(GF_give_stack, 4, w, to, idx, 9); else ns_call(GF_give_one, 4, w, to, idx, 9);
        if (changed(s0, snap(w, idx))) snprintf(t, sizeof t, all ? "%s gives the %s to %s." : "%s gives a %s to %s.", sheet_model.names[w], nm, sheet_model.names[to]);
        else snprintf(t, sizeof t, "%s cannot take the %s.", sheet_model.names[to], nm);
        break; }
    case SV_DROP:
        if (a->b == 255) ns_call(GF_item_remove_stack, 2, w, idx); else ns_call(GF_item_take_one, 2, w, idx);
        snprintf(t, sizeof t, a->b == 255 ? "Left behind: the %s (all of them)." : "Left behind: a %s.", nm);
        break;
    case SV_DRINK:
        if (!drinkable(g.id)) return;
        ns_call(GF_use_potion, 2, w, idx);
        if (ns_unwinding) return;
        ns_call(GF_item_take_one, 2, w, idx);
        snprintf(t, sizeof t, "%s drinks the %s.", sheet_model.names[w], nm);
        break;
    default: return;
    }
    if (ns_unwinding) return;
    doll_redraw(w);
    note(t);
}

static int prev_screen;
static void sheet_hook(void)                /* member_sheet(who), s031 1462:0124 */
{
    int who = (s16)rw(SS, (u16)(SP + 4));
    if (!sv_on || !ns_active() || who < 0 || who > 4) { hook_call_original(); return; }
    int st = gm_member_status(who);
    if (st == 0 || st == 5) { AX = 0xffff; hook_return_far(0); return; }
    /* the original's setup */
    g_wb(GV_font_flag_a, 2); g_wb(GV_font_flag_b, 2); g_wb(GV_screen_kind, 2);
    g_ww(GV_screen_win_colour, 0xbf);
    g_wb(GV_sheet_box, 0); g_wb(GV_sheet_who, (u8)who);
    ns_call(GF_party_panel_frames, 1, 0x100); ns_call(GF_party_panel_rows, 1, 0x100);
    ns_call(GF_sheet_open, 1, who);                      /* (fonts, the original's picture, unseen) */
    if (ns_unwinding) return;
    prev_screen = ns_screen;
    int prev_nested = ns_nested;
    memset(sheet_model.note, 0, sizeof sheet_model.note);
    build(who);
    sheet_model.open = 1;
    fprintf(stderr, "sheet: open (member %d, %d things)\n", who, sheet_model.nitems);
    ns_screen = NS_SHEET;
    ns_clear();
    ns_input_begin();
    int leave = 0;
    while (!leave) {
        if (!ns_round()) break;
        int cx, cy;
        if (ns_in.click && ns_in.cx < 0x3c) {                 /* the party panel: another member, or this one: close */
            ns_take_click(&cx, &cy);
            int m = cy / 40;
            if (m == who) leave = 1;
            else if (m >= 0 && m < 5) { NsAction a = { SV_WHO, m, 0, 0 }; act(&a, &who); build(who); }
        }
        if (g_rw(GV_party_icons_dirty)) {                                   /* party_icons_dirty */
            g_ww(GV_party_icons_dirty, 0);
            member_redraw(who);
            build(who);
        }
        if (ns_call(GF_key_pressed, 0)) {
            int k = (u16)ns_call(GF_get_key, 0);
            if (k == 0x11b || k == 0x1c0d) leave = 1;
            else if (k >= 0x3b00 && k <= 0x3f00 && !(k & 0xff)) { int m = (k >> 8) - 0x3b; if (m == who) leave = 1; else { NsAction a = { SV_WHO, m, 0, 0 }; act(&a, &who); build(who); } }
            else if (k >= 0x5e00 && k <= 0x6200 && !(k & 0xff)) { NsAction a = { SV_LEADER, (k >> 8) - 0x5e, 0, 0 }; act(&a, &who); build(who); }
            else if (k == 0x4800 || k == 0x5000) ns_in.wheel += k == 0x4800 ? 1 : -1;
            else if (k == 0x4900 || k == 0x5100) ns_in.wheel += k == 0x4900 ? 4 : -4;
            else if (k == 0x1000) { ns_call(GF_sheet_leave, 1, -1); ns_call(GF_quit_to_dos, 0); }
        }
        if (ns_unwinding) break;
        NsAction a;
        while (!leave && !ns_unwinding && ns_pop(&a)) {
            if (a.type == SV_CLOSE) { leave = 1; break; }
            act(&a, &who);
            if (!ns_unwinding) build(who);
        }
        if (ns_unwinding) break;
    }
    sheet_model.open = 0;
    fprintf(stderr, "sheet: closed\n");
    ns_screen = prev_screen;
    ns_nested = prev_nested;
    if (ns_unwinding) return;
    AX = (u16)ns_call(GF_sheet_leave, 1, -1);
    hook_return_far(0);
}

void sheetview_init(void)
{
    sv_on = config_get_int("display.native_sheet", 1);
    hook_install(GF_member_sheet, sheet_hook);
}
