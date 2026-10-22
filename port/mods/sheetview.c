/* sheetview.c - a member's sheet as a native screen (nscreen.h; the view is ui/sheetview.cpp,
 * which draws the whole page anew: the doll from the game's pictures, the things, the stats,
 * the party's saints and formulae).
 *
 * The original (member_sheet, s031 1462:0124) shows the portrait and boxes; the pack is a
 * 14-line box opened by clicking Equipment, worked with keys (A ready, U put away, 1..5 give,
 * D drop, P drink). Here the setup and the leaving are the original's own (09b8 / 08f6: fonts,
 * cursor, buffers; its picture is drawn but not shown), and the actions are the routines its
 * key handler calls:
 *   ready      s031 18e7:11b2 (who, idx)          put away  18e7:12e8 item_unequip (who, idx, 9)
 *   give one   18e7:0000 (from, to, idx, 9)       give all  18e7:007c give_stack
 *   drop one   18e7:0474 item_take_one (who, idx) drop all  18e7:07d4 item_remove_stack
 *   drink      1462:43b2 (who, idx), then item_take_one (as the original's P)
 *   take back what was dropped here: 18e7:016e add_item_q (who, item, quality), once per thing
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
void plat_hold(int ms);
#include "gamemodel.h"
#include "sheetview.h"
#include "keep.h"

SheetModel sheet_model;
int sheetview_dragging;
static int sv_on = 1;
u32 plat_ticks_ms(void);

int sheetview_is_on(void) { return sv_on; }
int sheetview_shared(void) { return config_get_int("inventory.shared", 1) && !config_get_int("original", 0); }
void sheetview_set(int on) { sv_on = on; config_set_int("display.native_sheet", on); }


static void note(const char *t) { snprintf(sheet_model.note, sizeof sheet_model.note, "%s", t); sheet_model.note_t = plat_ticks_ms(); fprintf(stderr, "sheet: %s\n", t); }

static int drinkable(int id)
{
    unsigned long c;
    return gm_itemdef_cats(id, &c) && (c & 0x800) && !(c & 0x1000000);
}

/* the statue each saint is shown with: its handler's class (handler (0, 0, 1), s039; the
 * prayer screen picks PICS\SNTBACK<class>.PIC by it), asked once, the game put back after */
static signed char saint_cls[160]; static int saint_cls_n = -1;
static void saint_classes(void)
{
    if (saint_cls_n >= 0) return;
    saint_cls_n = 0;
    memset(saint_cls, -1, sizeof saint_cls);
    u16 tseg = (u16)(LOADSEG + 0x290e), vseg = (u16)(LOADSEG + 0x09c0);
    void *snap = eval_snapshot();
    for (int t = 0; t < 160; t++) {
        u16 off = rw(tseg, (u16)(0x2937 + 4 * t)), seg = rw(tseg, (u16)(0x2937 + 4 * t + 2));
        if (seg != vseg || off < 0x1000) break;                 /* (past the last saint) */
        u16 a[3] = { 0, 0, 1 };
        u32 r = game_call_ptr(seg, off, 3, a, 0);
        eval_restore(snap);
        saint_cls[t] = (signed char)(game_call_timed_out ? -1 : (s16)r);
        saint_cls_n = t + 1;
    }
    eval_free(snap);
    fprintf(stderr, "sheet: %d saints' statues known\n", saint_cls_n);
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
        for (int k = 0; k < 66; k++) s->formula[m][k] = (unsigned char)(s->present[m] && gm_knows_formula(m, k));
        for (int k = 0; k < 160; k++) s->saint[m][k] = (unsigned char)(s->present[m] && gm_knows_saint(m, k));
        int c, mx;
        s->alch[m] = gm_member_skill(m, 7); s->relig[m] = gm_member_skill(m, 8);
        gm_member_attr(m, 4, &c, &mx); s->intel[m] = c;
        gm_member_attr(m, 6, &c, &mx); s->df[m] = c;
        for (int a = 0; a < 7; a++) { if (s->present[m]) gm_member_attr(m, a, &s->pattr[m][a], &s->pattr_max[m][a]); else s->pattr[m][a] = s->pattr_max[m][a] = 0; }
        for (int k = 0; k < 19; k++) s->pskill[m][k] = s->present[m] ? gm_member_skill(m, k) : 0;
    }
    memset(s->have, 0, sizeof s->have);
    memset(s->ingr, 0, sizeof s->ingr);
    for (int m = 0; m < s->npar && m < 5; m++)
        for (int i = 0; i < gm_item_count(m); i++) {
            GmItem g; if (!gm_item(m, i, &g) || g.id < 0 || g.id >= 200) continue;
            s->have[g.id] += g.count;
            if (g.id >= 72 && g.id <= 94) s->ingr[m][g.id - 72] += g.count;
        }
    gm_member_fullname(who, s->fullname, sizeof s->fullname);
    s->female = gm_member_female(who);
    s->age = gm_member_age(who);
    for (int a = 0; a < 7; a++) gm_member_attr(who, a, &s->attr[a], &s->attr_max[a]);
    for (int k = 0; k < 19; k++) s->skill[k] = gm_member_skill(who, k);
    s->load = gm_member_load(who);
    s->capacity = s->attr[0] + s->attr[1];
    s->load_band = gm_member_load_band(who);
    GmEquip e; gm_member_equip(who, &e);
    s->weapon = e.weapon; s->vital = e.vital; s->limb = e.limb; s->shield = e.shield; s->missile = e.missile;
    s->shared = sheetview_shared();
    s->nitems = 0;
    if (!s->shared) {
        for (int i = 0; i < gm_item_count(who) && i < 64; i++) {
            GmItem g; gm_item(who, i, &g);
            if (gm_ammo_hidden() && gm_is_ammo(g.id)) continue;   /* (arrows and the like never run out: not shown) */
            SvItem *it = &s->item[s->nitems++];
            it->idx = i; it->id = g.id; it->type = g.type; it->qual = g.qual; it->count = g.count; it->weight = g.weight;
            it->equipped = gm_item_equipped(who, i);
            it->drinkable = drinkable(g.id);
            it->holder = who; it->kept = keep_is(g.id, g.qual); it->potion = 0;
        }
    } else {
        /* the member's things in use, then their own potions, then the party's pack: every member's spares
           (but potions) by kind and quality */
        for (int i = 0; i < gm_item_count(who); i++) {
            GmItem g; if (!gm_item(who, i, &g) || !gm_item_in_use(who, i)) continue;
            SvItem *it = &s->item[s->nitems++];
            it->idx = i; it->id = g.id; it->type = g.type; it->qual = g.qual; it->count = 1; it->weight = g.weight;
            it->equipped = gm_item_equipped(who, i) ? 1 : 2;    /* (2: the mount they ride) */
            it->drinkable = drinkable(g.id); it->holder = who; it->kept = keep_is(g.id, g.qual); it->potion = 0;
        }
        for (int i = 0; i < gm_item_count(who) && s->nitems < 200; i++) {
            GmItem g; if (!gm_item(who, i, &g) || gm_item_in_use(who, i) || !gm_is_potion(g.id)) continue;
            SvItem *it = &s->item[s->nitems++];
            it->idx = i; it->id = g.id; it->type = g.type; it->qual = g.qual; it->count = g.count; it->weight = g.weight;
            it->equipped = 0; it->drinkable = drinkable(g.id); it->holder = who; it->kept = keep_is(g.id, g.qual); it->potion = 1;
        }
        int first = s->nitems;
        for (int m = 0; m < 5; m++) {
            if (!gm_member_present(m) || gm_member_status(m) == 5) continue;
            for (int i = 0; i < gm_item_count(m); i++) {
                GmItem g; if (!gm_item(m, i, &g) || gm_is_potion(g.id) || (gm_ammo_hidden() && gm_is_ammo(g.id))) continue;
                int spare = gm_item_spare(m, i);
                if (spare <= 0) continue;
                int k = first;
                while (k < s->nitems && !(s->item[k].id == g.id && s->item[k].qual == g.qual)) k++;
                if (k == s->nitems) {
                    if (s->nitems >= 200) continue;
                    SvItem *it = &s->item[s->nitems++];
                    it->idx = -1; it->id = g.id; it->type = g.type; it->qual = g.qual; it->count = 0; it->weight = g.weight;
                    it->equipped = 0; it->drinkable = drinkable(g.id); it->holder = -1; it->kept = keep_is(g.id, g.qual); it->potion = 0;
                }
                s->item[k].count += spare;
            }
        }
        for (int a = first + 1; a < s->nitems; a++) {           /* (by kind in the game's list order, then the better first) */
            SvItem t = s->item[a]; int b = a - 1;
            while (b >= first && (s->item[b].id > t.id || (s->item[b].id == t.id && s->item[b].qual < t.qual))) { s->item[b + 1] = s->item[b]; b--; }
            s->item[b + 1] = t;
        }
    }
    memcpy(s->saint_class, saint_cls, sizeof s->saint_class);
    s->nsaints = saint_cls_n;
    s->seq++;
}

static void item_name(int id, char *out, int n) { gm_itemdef_name(id, out, n); }

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

static int pack_holder(int prefer, int id, int q, int *m_out, int *i_out) { return gm_pack_find(prefer, id, q, m_out, i_out); }
static int record_of(int m, int id, int q, int unequipped_first)
{
    int any = -1;
    for (int i = 0; i < gm_item_count(m); i++) {
        GmItem g; if (!gm_item(m, i, &g) || g.id != id || g.qual != q) continue;
        if (!unequipped_first || !gm_item_equipped(m, i) || g.count > 1) return i;
        any = i;
    }
    return any;
}
/* one of the pack's (id, q) in member w's hands: their record of it, -1 if it could not be handed over */
static int pack_fetch(int w, int id, int q, const char *nm, char *t, size_t tn)
{
    int h, i;
    if (!pack_holder(w, id, q, &h, &i)) { snprintf(t, tn, "No %s is left in the pack.", nm); return -1; }
    if (h != w) {
        if (gm_item_count(w) >= 64 && record_of(w, id, q, 0) < 0) { snprintf(t, tn, "%s cannot carry another kind of item.", sheet_model.names[w]); return -1; }
        ns_call(GF_give_one, 4, h, w, i, 9);
        if (ns_unwinding) return -1;
    }
    int r = record_of(w, id, q, 1);
    if (r < 0) snprintf(t, tn, "The %s could not be handed to %s.", nm, sheet_model.names[w]);
    return r;
}

static void act(const NsAction *a, int *who)
{
    int w = *who, idx = a->a;
    GmItem g; char nm[32] = "", t[160];
    if (a->type == SV_PREADY || a->type == SV_PDRINK || a->type == SV_PDROP || a->type == SV_KEEP) {
        int id = a->a, q = a->b;
        item_name(id, nm, sizeof nm);
        switch (a->type) {
        case SV_KEEP:
            keep_set(id, q, a->c);
            snprintf(t, sizeof t, a->c ? "The %s is marked Keep: it will not be sold or dropped until you unmark it." : "The %s may be sold and dropped again.", nm);
            break;
        case SV_PREADY: {
            int r = pack_fetch(w, id, q, nm, t, sizeof t);
            if (ns_unwinding) return;
            if (r < 0) break;
            Snap s0 = snap(w, r);
            ns_call(GF_item_ready, 2, w, r);
            if (ns_unwinding) return;
            if (changed(s0, snap(w, r))) snprintf(t, sizeof t, "%s takes up the %s.", sheet_model.names[w], nm);
            else snprintf(t, sizeof t, "The %s cannot be readied.", nm);
            break; }
        case SV_PDRINK: {
            if (!drinkable(id)) return;
            int r = pack_fetch(w, id, q, nm, t, sizeof t);
            if (ns_unwinding) return;
            if (r < 0) break;
            ns_call(GF_use_potion, 2, w, r);
            if (ns_unwinding) return;
            ns_call(GF_item_take_one, 2, w, r);
            snprintf(t, sizeof t, "%s drinks the %s.", sheet_model.names[w], nm);
            break; }
        case SV_PDROP: {
            if (keep_is(id, q)) { snprintf(t, sizeof t, "The %s is marked Keep: unmark it before dropping it.", nm); break; }
            int want = a->c == 255 ? 9999 : 1, done = 0, h, i;
            while (done < want && pack_holder(-1, id, q, &h, &i)) {
                GmItem g2; gm_item(h, i, &g2);
                int spare = gm_item_spare(h, i);
                if (!gm_item_in_use(h, i) && want - done >= g2.count) { ns_call(GF_item_remove_stack, 2, h, i); done += g2.count; }
                else { ns_call(GF_item_take_one, 2, h, i); done++; }
                if (ns_unwinding) return;
                (void)spare;
            }
            SheetModel *s = &sheet_model;
            if (done) {
                if (s->ndrops == 16) { memmove(&s->drops[0], &s->drops[1], 15 * sizeof s->drops[0]); s->ndrops--; }
                SvDrop *d = &s->drops[s->ndrops++];
                d->id = id; d->qual = q; d->count = done; snprintf(d->name, sizeof d->name, "%s", nm);
            }
            snprintf(t, sizeof t, done > 1 ? "Left behind: the %s (%d). Undo takes them back." : "Left behind: a %s. Undo takes it back.", nm, done);
            break; }
        }
        if (ns_unwinding) return;
        note(t);
        return;
    }
    if (a->type == SV_PRAY) {                   /* the original sheet's saints list: a saint clicked calls on it (saint_pray) */
        int m = a->a, t = a->b;
        if (!gm_member_present(m) || gm_member_status(m) != 1 || !gm_knows_saint(m, t)) return;
        ns_call(GF_saint_pray, 2, m, t);
        if (ns_unwinding) return;
        member_redraw(*who);
        return;
    }
    if (a->type != SV_WHO && a->type != SV_LEADER && a->type != SV_CLOSE && a->type != SV_UNDO) {
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
        if (gm_member_present(a->a) && gm_member_status(a->a) == 1) { g_wb(G_LEADER, (u8)a->a); ns_call(GF_party_panel_rows, 1, 0x100); }
        return;
    case SV_UNDO: {
        SheetModel *s = &sheet_model;
        if (s->ndrops <= 0) return;
        SvDrop d = s->drops[--s->ndrops];
        int got = 0;
        for (int k = 0; k < d.count && !ns_unwinding; k++) {
            int before = gm_item_count(w), had = 0;
            for (int i = 0; i < before; i++) { GmItem g; if (gm_item(w, i, &g) && g.id == d.id && g.qual == d.qual) had += g.count; }
            ns_call(GF_add_item_q, 3, w, d.id, d.qual);
            int now = 0;
            for (int i = 0; i < gm_item_count(w); i++) { GmItem g; if (gm_item(w, i, &g) && g.id == d.id && g.qual == d.qual) now += g.count; }
            if (now <= had) break;
            got++;
        }
        if (ns_unwinding) return;
        if (got) snprintf(t, sizeof t, got > 1 ? "Taken back: the %s (%d)." : "Taken back: the %s.", d.name, got);
        else snprintf(t, sizeof t, "The %s cannot be taken back (no room).", d.name);
        note(t);
        return; }
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
        if (before >= 64 && !all) { snprintf(t, sizeof t, "%s cannot carry another kind of item.", sheet_model.names[to]); break; }
        if (all) ns_call(GF_give_stack, 4, w, to, idx, 9); else ns_call(GF_give_one, 4, w, to, idx, 9);
        if (changed(s0, snap(w, idx))) snprintf(t, sizeof t, all ? "%s gives the %s to %s." : "%s gives a %s to %s.", sheet_model.names[w], nm, sheet_model.names[to]);
        else snprintf(t, sizeof t, "%s cannot take the %s.", sheet_model.names[to], nm);
        break; }
    case SV_DROP: {
        SheetModel *s = &sheet_model;
        if (keep_is(g.id, g.qual)) { snprintf(t, sizeof t, "The %s is marked Keep: unmark it before dropping it.", nm); break; }
        int n = a->b == 255 ? g.count : 1;
        if (a->b == 255) ns_call(GF_item_remove_stack, 2, w, idx); else ns_call(GF_item_take_one, 2, w, idx);
        if (ns_unwinding) return;
        if (s->ndrops == 16) { memmove(&s->drops[0], &s->drops[1], 15 * sizeof s->drops[0]); s->ndrops--; }
        SvDrop *d = &s->drops[s->ndrops++];
        d->id = g.id; d->qual = g.qual; d->count = n; snprintf(d->name, sizeof d->name, "%s", nm);
        snprintf(t, sizeof t, n > 1 ? "Left behind: the %s (%d). Undo takes them back." : "Left behind: a %s. Undo takes it back.", nm, n);
        break; }
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
    note(t);
}

static void sheet_hook(void)                /* member_sheet(who), s031 1462:0124 */
{
    int who = (s16)rw(SS, (u16)(SP + 4));
    if (!sv_on || !ns_active() || who < 0 || who > 4) { hook_call_original(); return; }
    plat_hold(600);                              /* (the old screen drawn while it is set up: not shown) */
    int st = gm_member_status(who);
    if (st == 0 || st == 5) { AX = 0xffff; hook_return_far(0); return; }
    /* the original's setup */
    g_wb(GV_font_flag_a, 2); g_wb(GV_font_flag_b, 2); g_wb(GV_screen_kind, 2);
    g_ww(GV_screen_win_colour, 0xbf);
    g_wb(GV_sheet_box, 0); g_wb(GV_sheet_who, (u8)who);
    ns_call(GF_party_panel_frames, 1, 0x100); ns_call(GF_party_panel_rows, 1, 0x100);
    ns_call(GF_sheet_open, 1, who);                      /* (fonts, the original's picture, unseen) */
    if (ns_unwinding) return;
    memset(sheet_model.note, 0, sizeof sheet_model.note);
    sheet_model.ndrops = 0;
    saint_classes();
    build(who);
    sheet_model.open = 1; sheet_model.tab_step = 0;
    fprintf(stderr, "sheet: open (member %d, %d things)\n", who, sheet_model.nitems);
    ns_enter(NS_SHEET);
    ns_clear();
    ns_input_begin();
    int leave = 0;
    while (!leave) {
        if (!ns_round()) break;
        int cx, cy;
        if (ns_in.click && ns_in.cx < 0x3c && !sheetview_dragging) {   /* the party panel: another member, or this one: close (not the end of a drag: a give) */
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
            int k = ns_get_key();
            if (k == 0x11b || k == 0x1c0d) leave = 1;
            else if (k >= 0x3b00 && k <= 0x3f00 && !(k & 0xff)) { int m = (k >> 8) - 0x3b; if (m == who) leave = 1; else { NsAction a = { SV_WHO, m, 0, 0 }; act(&a, &who); build(who); } }
            else if (k >= 0x5e00 && k <= 0x6200 && !(k & 0xff)) { NsAction a = { SV_LEADER, (k >> 8) - 0x5e, 0, 0 }; act(&a, &who); build(who); }
            else if (k == 0x4800 || k == 0x5000) ns_in.wheel += k == 0x4800 ? 1 : -1;
            else if (k == 0x0f09 || k == 0x4d00) sheet_model.tab_step++;          /* (Tab, Right: the next page) */
            else if (k == 0x0f00 || k == 0x4b00) sheet_model.tab_step--;          /* (Shift+Tab, Left) */
            else if (k == 0x4900 || k == 0x5100) ns_in.wheel += k == 0x4900 ? 4 : -4;
            else if (k == 0x1000) { ns_call(GF_sheet_leave, 1, -1); ns_call(GF_quit_to_dos, 0); }
            else if (((k & 0xff) >= '1' && (k & 0xff) <= '5') || ((k >> 8) >= 2 && (k >> 8) <= 6 && (k & 0xff) && strchr("!@#$%", k & 0xff))) {
                /* 1..5: the thing under the pointer to that member, the whole stack (Shift: one) */
                int sheetview_pointed_item(void);
                int r = sheetview_pointed_item(), to = (k >> 8) - 2, one = !((k & 0xff) >= '1' && (k & 0xff) <= '5');
                const SheetModel *sm = &sheet_model;
                if (r >= 0 && r < sm->nitems && sm->item[r].holder >= 0 && !sm->item[r].equipped && to != who) {
                    NsAction g = { SV_GIVE, sm->item[r].idx, to, one ? 1 : 255 }; act(&g, &who); build(who);
                }
            }
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
    ns_leave();
    if (ns_unwinding) return;
    AX = (u16)ns_call(GF_sheet_leave, 1, -1);
    hook_return_far(0);
}

void sheetview_init(void)
{
    sv_on = config_get_int("display.native_sheet", 1);
    hook_install(GF_member_sheet, sheet_hook);
}

/* test: member m prays to saint s (as the sheet's "X prays") */
void sheetview_test_pray(int m, int s)
{
    if (s < 0) for (int k = 0; k < 140 && s < 0; k++) if (gm_knows_saint(m, k)) s = k;   /* (-1: the first saint the member knows) */
    fprintf(stderr, "test: member %d prays to %d\n", m, s);
    ns_push(SV_PRAY, m, s, 0);
}
