/* menuview.c - the game's menu bar (Game, Orders, Attack, Party on the battle field; the menus of
 * the map and the cards) drawn and used by Darklands+ (display.native_menu; "Play the original
 * game" keeps the game's). The view is ui/menuview.cpp.
 *
 * The original (menu_bar_run, root 0000:0000, called by every screen's loop) shows the bar only
 * while the right button is held: the player drags to an entry and lets go on it (or presses F10
 * and steps through with the arrow keys). Its entries are the game's own tables (struct menu at
 * DS:9419, 10 bytes: name, x0, x1, has_items (1 open), items; struct item, 0x14 bytes: name (a
 * byte 1 for a tick or 2 before the text), enabled (1), shortcut, handler far +0x0e, sub-menu
 * +0x12; sub-items 0x16 bytes: name, enabled, shortcut[4], count +7, handler +0x12), which the
 * game keeps up to date (who is selected, what can be done now).
 *
 * Here the same tables are shown as a menu that stays open: a right click (or F10) opens it,
 * a click on an entry chooses it (letting go of a held right button over one still does), a
 * click elsewhere, Esc or F10 closes it. The chosen entry's own handler runs, exactly as the
 * original calls it. While the menu is open the game waits, as it does under the original's. */
#include <stdio.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../runtime/hle.h"
#include "nscreen.h"
#include "menuview.h"

#define D_KEYMODE   0x9c26          /* char: the keyboard steps through the menus (F10) */
#define D_ITEM_HIT  0x944b          /* char: the pointer is on an item (the original's release chooses it) */
#define D_SUB_HIT   0x9783          /* char: on a sub-item */
#define ARG(k) rw(SS, (u16)(SP + 4 + 2 * (k)))

u32 plat_ticks_ms(void);
void plat_sleep_ms(int ms);
void dl_poll(void);
void native_go(u16 seg, u16 off);
unsigned mouse_press_count(int b);

MenuModel menu_model;
NsInput mv_in;                      /* the pointer's clicks while the menu is open (the views under it get none) */
static int mv_on = 1;

/* Darklands+'s own menu: its entries' actions (in place of a game handler: segment NATIVE_SEG) */
#define NATIVE_SEG 0xffff
enum { NA_WINDOW = 1, NA_QUESTS, NA_MAP, NA_HELPERS, NA_GUIDE, NA_HELP };
#include "../ui/uitabs.h"
void ui_open_tab(int t);
void ui_toggle_map(void);
void ui_toggle_help(void);
static void native_action(int a)
{
    switch (a) {
    case NA_WINDOW: ui_open_tab(-1); break;
    case NA_QUESTS: ui_open_tab(UI_PAGE_SUB(UI_PAGE_JOURNAL, UI_JOURNAL_QUESTS)); break;
    case NA_MAP: ui_toggle_map(); break;
    case NA_HELPERS: ui_open_tab(UI_PAGE_HELPERS); break;
    case NA_GUIDE: ui_open_tab(UI_PAGE_GUIDE); break;
    case NA_HELP: ui_toggle_help(); break;
    }
}
typedef struct { u16 seg, off; } MvFn;
static int call_handler(u16 seg, u16 off);
static MvFn fn[96];

int menuview_is_on(void) { return mv_on; }
void menuview_set(int on) { mv_on = on; config_set_int("display.native_menu", on); }
int menuview_open(void) { return menu_model.open; }

#define NQ 16
static NsAction q[NQ]; static int qh, qt;
void menuview_push(int type, int a, int b, int c) { int n = (qt + 1) % NQ; if (n == qh) return; q[qt].type = type; q[qt].a = a; q[qt].b = b; q[qt].c = c; qt = n; }
static int pop(NsAction *o) { if (qh == qt) return 0; *o = q[qh]; qh = (qh + 1) % NQ; return 1; }

/* ---------------------------------------------------------------- the game's tables */
static int str_at(u16 p, char *out, int n)            /* a DGROUP string; its length */
{
    int i = 0;
    if (p) for (; i < n - 1; i++) { char c = (char)g_rb((u16)(p + i)); if (!c) break; out[i] = c; }
    out[i] = 0;
    return i;
}
/* the game's font has signs for keys: 0x05..0x0e F1..F10, 0x0f Alt, 0x12 Return, 0x13 Space */
static void key_text(u16 p, int maxn, char *out, int n)
{
    out[0] = 0;
    for (int i = 0; i < maxn; i++) {
        int c = g_rb((u16)(p + i));
        if (!c) break;
        char b[12] = "";
        if (c >= 0x05 && c <= 0x0e) snprintf(b, sizeof b, "F%d", c - 4);
        else if (c == 0x0f) snprintf(b, sizeof b, "Alt+");
        else if (c == 0x10) snprintf(b, sizeof b, "Ctrl+");
        else if (c == 0x11) snprintf(b, sizeof b, "Shift+");
        else if (c == 0x12) snprintf(b, sizeof b, "Enter");
        else if (c == 0x13) snprintf(b, sizeof b, "Space");
        else if (c == '>' || c == ' ') continue;          /* (a sub-menu's sign; the padding) */
        else if (c > 32 && c < 127) { b[0] = (char)c; b[1] = 0; }
        strncat(out, b, (size_t)n - strlen(out) - 1);
    }
}
static void item_name(u16 p, MvItem *it)
{
    char s[64]; int n = str_at(p, s, sizeof s);
    const char *t = s;
    it->tick = 0;
    if (n && (s[0] == 1 || s[0] == 2)) { it->tick = s[0] == 1; t++; }
    snprintf(it->name, sizeof it->name, "%s", t);
    for (int k = (int)strlen(it->name) - 1; k >= 0 && it->name[k] == ' '; k--) it->name[k] = 0;
}

static void build_into(u16 mp, MenuModel *M, MvFn *fn)
{
    M->nmenus = 0; M->nitems = 0;
    for (int j = 0; j < 6; j++) {
        u16 e = (u16)(mp + 10 * j);
        char nm[24];
        if (!str_at(g_rw(e), nm, sizeof nm)) break;
        MvMenu *m = &M->menu[M->nmenus++];
        snprintf(m->name, sizeof m->name, "%s", nm);
        m->enabled = g_rb((u16)(e + 6)) == 1;
        m->first = M->nitems; m->n = 0;
        u16 items = g_rw((u16)(e + 8));
        for (int k = 0; items && k < 24 && M->nitems < 96; k++) {
            u16 b = (u16)(items + 0x14 * k);
            char t[4];
            if (!str_at(g_rw(b), t, sizeof t)) break;
            int ix = M->nitems++;
            MvItem *it = &M->items[ix];
            item_name(g_rw(b), it);
            it->enabled = g_rb((u16)(b + 2)) == 1;
            key_text((u16)(b + 3), 0x0b, it->key, sizeof it->key);
            it->count = 0; it->nsub = -1;
            fn[ix].off = g_rw((u16)(b + 0x0e)); fn[ix].seg = g_rw((u16)(b + 0x10));
            M->sub_first[ix] = 0; M->sub_n[ix] = 0;
            m->n++;
        }
    }
    /* the sub-menus, after all the items (an item's sub-items: sub_first / sub_n) */
    for (int j = 0; j < M->nmenus; j++) {
        u16 items = g_rw((u16)(mp + 10 * j + 8));
        for (int k = 0; k < M->menu[j].n; k++) {
            int ix = M->menu[j].first + k;
            u16 subs = g_rw((u16)(items + 0x14 * k + 0x12));
            if (!subs) continue;
            M->sub_first[ix] = M->nitems;
            for (int s = 0; s < 24 && M->nitems < 96; s++) {
                u16 b = (u16)(subs + 0x16 * s);
                char t[4];
                if (!str_at(g_rw(b), t, sizeof t)) break;
                int sx = M->nitems++;
                MvItem *it = &M->items[sx];
                item_name(g_rw(b), it);
                it->enabled = g_rb((u16)(b + 2)) == 1;
                key_text((u16)(b + 3), 4, it->key, sizeof it->key);
                it->count = g_rb((u16)(b + 7));
                it->nsub = -1;
                fn[sx].off = g_rw((u16)(b + 0x12)); fn[sx].seg = g_rw((u16)(b + 0x14));
                M->sub_first[sx] = 0; M->sub_n[sx] = 0;
                M->sub_n[ix]++;
            }
            if (M->sub_n[ix]) M->items[ix].nsub = ix;
        }
    }
    for (int j = 0; j < M->nmenus; j++) M->menu[j].native = 0;
    /* Darklands+'s own menu, at the right end: its window and pages (handled here, not the game's) */
    static const struct { const char *name, *key; int act; } DP[] = {
        { "Darklands+ window", "F12", NA_WINDOW }, { "Quests", "", NA_QUESTS }, { "World map", "M", NA_MAP },
        { "Helpers (settings)", "", NA_HELPERS }, { "Guide", "", NA_GUIDE }, { "Keys and clicks here", "F1", NA_HELP },
    };
    int nd = (int)(sizeof DP / sizeof DP[0]);
    if (M->nmenus < 7 && M->nitems + nd <= 96) {
        MvMenu *m = &M->menu[M->nmenus++];
        snprintf(m->name, sizeof m->name, "Darklands+");
        m->enabled = 1; m->native = 1; m->first = M->nitems; m->n = 0;
        for (int k = 0; k < nd; k++) {
            int ix = M->nitems++;
            MvItem *it = &M->items[ix];
            snprintf(it->name, sizeof it->name, "%s", DP[k].name); snprintf(it->key, sizeof it->key, "%s", DP[k].key);
            it->enabled = 1; it->tick = 0; it->count = 0; it->nsub = -1;
            fn[ix].seg = NATIVE_SEG; fn[ix].off = (u16)DP[k].act;
            M->sub_first[ix] = 0; M->sub_n[ix] = 0;
            m->n++;
        }
    }
}
static void build(u16 mp) { build_into(mp, &menu_model, fn); }

/* the menus' entries for buttons elsewhere (the battle margin's orders): read now; a choice is
   carried out at the next round of the game's loop (menu_bar_run), if still allowed then */
static MenuModel live; static MvFn live_fn[96];
static int req_menu = -1, req_item, req_sub;
const MenuModel *menuview_live(void) { build_into(0x9419, &live, live_fn); return &live; }
void menuview_request(int m, int i, int s) { req_menu = m; req_item = i; req_sub = s; }
static int run_request(void)
{
    int m = req_menu, i = req_item, s = req_sub;
    req_menu = -1;
    build_into(0x9419, &live, live_fn);
    if (m < 0 || m >= live.nmenus || !live.menu[m].enabled || i < 0 || i >= live.menu[m].n) return 1;
    int ix = live.menu[m].first + i;
    if (s >= 0) { if (s >= live.sub_n[ix]) return 1; ix = live.sub_first[ix] + s; }
    if (!live.items[ix].enabled) return 1;
    fprintf(stderr, "menu: %s > %s (a button)\n", live.menu[m].name, live.items[ix].name);
    return call_handler(live_fn[ix].seg, live_fn[ix].off);
}

/* ---------------------------------------------------------------- the keyboard */
static int item_ix(int m, int r) { return menu_model.menu[m].first + r; }
static int has_sub(int m, int r) { return r >= 0 && menu_model.sub_n[item_ix(m, r)] > 0; }
static int next_menu(int from, int dir)
{
    MenuModel *M = &menu_model;
    for (int k = 1; k <= M->nmenus; k++) {
        int j = ((from + dir * k) % M->nmenus + M->nmenus) % M->nmenus;
        if (M->menu[j].enabled) return j;
    }
    return from;
}
static int next_row(int n, int first, int from, int dir)
{
    for (int k = 1; k <= n; k++) {
        int r = from + dir * k;
        if (r < 0 || r >= n) break;
        if (menu_model.items[first + r].enabled) return r;
    }
    return from;
}
/* 1: chosen (menu_model.cur/row/subrow), -1: closed, 0: on */
static int key(int k)
{
    MenuModel *M = &menu_model;
    if (M->cur < 0) M->cur = next_menu(-1, 1);
    MvMenu *m = &M->menu[M->cur];
    switch (k) {
    case 0x011b: case 0x4400: return -1;                       /* Esc, F10 */
    case 0x1c0d:                                                /* Enter */
        if (M->row < 0) { M->row = next_row(m->n, m->first, -1, 1); return 0; }
        if (M->subrow >= 0) return M->items[M->sub_first[item_ix(M->cur, M->row)] + M->subrow].enabled ? 1 : 0;
        if (has_sub(M->cur, M->row)) { M->subrow = next_row(M->sub_n[item_ix(M->cur, M->row)], M->sub_first[item_ix(M->cur, M->row)], -1, 1); return 0; }
        return M->items[item_ix(M->cur, M->row)].enabled ? 1 : 0;
    case 0x4800: case 0x5000: {                                 /* Up, Down */
        int d = k == 0x4800 ? -1 : 1;
        if (M->subrow >= 0) { int ix = item_ix(M->cur, M->row); M->subrow = next_row(M->sub_n[ix], M->sub_first[ix], M->subrow, d); }
        else M->row = next_row(m->n, m->first, M->row < 0 && d < 0 ? m->n : M->row, d);
        return 0;
    }
    case 0x4b00:                                                /* Left */
        if (M->subrow >= 0) { M->subrow = -1; return 0; }
        M->cur = next_menu(M->cur, -1); M->row = -1; return 0;
    case 0x4d00:                                                /* Right */
        if (M->subrow < 0 && has_sub(M->cur, M->row) && M->items[item_ix(M->cur, M->row)].enabled) {
            int ix = item_ix(M->cur, M->row);
            M->subrow = next_row(M->sub_n[ix], M->sub_first[ix], -1, 1); return 0;
        }
        M->cur = next_menu(M->cur, 1); M->row = -1; M->subrow = -1; return 0;
    }
    return 0;
}

/* ---------------------------------------------------------------- the loop */
static int call_handler(u16 seg, u16 off)              /* as the original calls it; 0: the game unwinds */
{
    if (!seg && !off) return 1;
    if (seg == NATIVE_SEG) { native_action(off); return 1; }
    ns_nested = 1;                                      /* (a native screen under the menu hides while it runs) */
    u16 sp0 = SP, ds = DS, es = ES;
    DS = ES = G_DGROUP;
    tail_pending = 0;
    push16(HLE_SEG); push16(0xfff0);
    native_go(seg, off);
    ns_nested = 0;
    if (unw_active) { ns_unwinding = 1; return 0; }
    SP = sp0; DS = ds; ES = es;
    return 1;
}

static int run(u16 mp, int keyboard)
{
    MenuModel *M = &menu_model;
    build(mp);
    if (!M->nmenus) return 1;
    M->keyboard = keyboard;
    M->held = !keyboard && (g_rb(GV_mouse_buttons) & 2);
    M->cur = keyboard ? next_menu(-1, 1) : -1; M->row = -1; M->subrow = -1;
    g_wb(D_KEYMODE, 0); g_wb(D_ITEM_HIT, 0); g_wb(D_SUB_HIT, 0);
    qh = qt = 0;
    M->open = 1; M->seq++; M->t_open = plat_ticks_ms();
    int nested = ns_nested; unsigned since = ns_nested_since;
    ns_nested = 0;                                      /* (the native screen under it stays on show) */
    g_ww(GV_menu_bar_active, 1);
    ns_input_begin();
    memset(&mv_in, 0, sizeof mv_in);
    int chosen = 0;
    while (!chosen) {
        dl_poll();
        plat_sleep_ms(3);
        ns_input_poll();
        /* the clicks are the menu's (kept until the view, drawn less often than this runs, takes them) */
        mv_in.x = ns_in.x; mv_in.y = ns_in.y; mv_in.down = ns_in.down;
        if (ns_in.click) { mv_in.click = 1; mv_in.cx = ns_in.cx; mv_in.cy = ns_in.cy; }
        if (ns_in.rclick) mv_in.rclick = 1;
        ns_in.click = ns_in.rclick = ns_in.pressed = 0;
        while (ns_call(GF_key_pressed, 0)) {
            int k = ns_call(GF_get_key, 0) & 0xffff;
            int r = key(k);
            if (r) { chosen = r; break; }
        }
        NsAction a;
        while (!chosen && pop(&a)) {
            if (a.type == MV_CLOSE) chosen = -1;
            else if (a.type == MV_CHOOSE) { M->cur = a.a; M->row = a.b; M->subrow = a.c; chosen = 1; }
        }
    }
    M->open = 0;
    g_ww(GV_menu_bar_active, 0);
    while (g_rb(GV_mouse_buttons) & 2) { dl_poll(); plat_sleep_ms(3); }   /* (a held button would open it again) */
    ns_input_begin();
    ns_nested = nested; ns_nested_since = since;
    if (chosen != 1 || M->cur < 0 || M->row < 0) return 1;
    int ix = item_ix(M->cur, M->row);
    if (M->subrow >= 0) ix = M->sub_first[ix] + M->subrow;
    if (!M->items[ix].enabled) return 1;
    fprintf(stderr, "menu: %s > %s\n", M->menu[M->cur].name, M->items[ix].name);
    return call_handler(fn[ix].seg, fn[ix].off);
}

static void bar_hook(void)                       /* menu_bar_run (menus, once) */
{
    u16 mp = ARG(0); int once = (s16)ARG(1);
    if (!mv_on || once || !ns_active()) { hook_call_original(); return; }
    int first = 0;
    if (ns_call(GF_key_pressed, 0)) {                   /* the hot keys (F10 starts the keyboard's way through the menus) */
        ns_call(GF_hot_keys, 0);
        if (ns_unwinding) return;
    }
    if (!g_rb(GV_pause_depth)) { g_wb(GV_pause_depth, 1); first = 1; }
    int kb = g_rb(D_KEYMODE) == 1;
    /* a right click since the last time (the travel map's loop waits between steps: a quick click
       can come and go in between) */
    static unsigned rp_seen; unsigned rp = mouse_press_count(1);
    int rclicked = rp != rp_seen; rp_seen = rp;
    if (kb || rclicked || (g_rb(GV_mouse_buttons) & 2)) {
        if (!run(mp, kb)) return;                       /* (the game unwinds: no return of ours) */
    } else if (req_menu >= 0 && mp == 0x9419) {
        if (!run_request()) return;
    }
    if (--first == 0) g_wb(GV_pause_depth, 0);
    hook_return_far(0);
}

void menuview_init(void)
{
    mv_on = config_get_int("display.native_menu", 1);
    hook_install(GF_menu_bar_run, bar_hook);
}
