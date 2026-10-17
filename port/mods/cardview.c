/* cardview.c - the card screen drawn by Darklands+ itself.
 *
 * The game's card engine (s043 150b, src/wip/s043/s043_150b.c) still decides everything: the
 * scripts load a card, pick its text blocks, switch options on and off and act on the choice.
 * What is replaced here is only how a text is shown and how an option is chosen:
 *
 *  - card_text (03b0) and card_options (049a) show the picture as the original does (its
 *    palette, frame and the picture copied into the card window), expand the text block
 *    ($variables, text_expand 06fe) into msg_buf and read it into pieces (paragraphs and
 *    option lines, with the original's rules: option codes 15/10/16/06, opt_on[] 0 hidden /
 *    1 choosable / 2 shown, conditional parts 01..13 by D_9c30[]), but draw no text;
 *  - card_text then waits as wait_continue (2826) does, card_menu (1142, through mods/assist.c's
 *    hook) as the original menu loop does: the menu bar, the party's portraits, F1..F6 and
 *    Shift-F1..F5, Alt-Q, Up/Down/Enter - only the options are found under the pointer from
 *    the native layout (ui/cardview.cpp) instead of the picture's lines;
 *  - the redraw after a member's sheet (13f2) puts the picture back without the old text.
 *
 * The option layout D_e96e (lines, option numbers, kinds, y/h) is filled from the native
 * layout, so the parts of Darklands+ that read it (chances, number keys, Go to) keep working.
 * Clicks are the game's own mouse (mouse_buttons, mouse_x/y): the game controller, Go to and
 * the test scripts click the native options as they clicked the old ones. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../runtime/hle.h"
#include "cardview.h"
#include "fixes.h"
#include "nscreen.h"

CardView cardview;
int cardview_on = 1;

int plat_headless(void);
u32 plat_ticks_ms(void);
void plat_sleep_ms(int ms);
void plat_mouse_game(int *x, int *y);
void dl_poll(void);

/* the card engine's variables (DGROUP) */
#define A_CARD_WIN    GV_card_window
#define A_SCREEN      GV_screen_ptr
#define A_COMBAT_CLK  GV_combat_clock
#define A_MOUSE_BTN   GV_mouse_buttons
#define A_MENUS       0x9419
#define A_ICONS_DIRTY GV_party_icons_dirty
#define A_9C4C        0x9c4c
#define A_CONDS       0x9c30          /* char[]: conditional text parts shown (01..13) */
#define A_MSG_BUF     GV_msg_buf
#define A_8DDE        0x8dde          /* a choice was clicked */
#define A_8DDC        0x8ddc
#define A_MENUBAR_ON  GV_menu_bar_active          /* the menu bar is open */
#define A_PALETTE     GV_cur_palette
#define A_E898        0xe898          /* 0 text, 1 option list, 2 option list faded (for 13f2) */
#define A_PICNAME     0xee5b
#define A_TEXTCOL     0xee59          /* 0 dark text, 15 light text (a picture whose name starts with x) */
#define A_E959        0xe959          /* text block shown */
#define A_EE55        0xee55          /* the picture's palette is not set yet */
#define A_LAYOUT      GV_card_layout          /* struct layout: n, highlight, w, x, y[20], h[20], en[20], opt[20], kind[20] */
#define A_HILITE      GV_menu_highlight
#define A_OPT_ON      GV_opt_on
#define A_EE41        GV_card_box_shown
#define A_EE42        GV_card_box_pointer
#define A_MENU_MEMBER GV_menu_member
#define A_MENU_SLOT   GV_menu_slot
#define A_MENU_POPUP  GV_menu_popup
#define A_ED6C        0xed6c
#define A_BLK_X       0xe8d9          /* int[25] */
#define A_BLK_Y       0xe90b          /* u8[26] */
#define A_BLK_W       0xe925          /* int[25] */
#define A_EA92        0xea92          /* char[5][4]: the pop-up's en[] (D_ea64 + 0x2e): 1 choosable, 2 shown, 0 not shown */
#define A_POPUP_LAY   0xea64          /* struct layout of the pop-up; its +0xa6 = D_eb0a: the handlers per slot */
#define A_POP_ITEMS   0xeb5a          /* the items' and the saints' pop-up layouts (+0xa6: item_fn / saint_fn) */
#define A_POP_SAINTS  0xec50
#define A_CARD_ITEMS  GV_card_items
#define A_CARD_SAINTS GV_card_saints

/* ---------------------------------------------------------------- calling the game */
#define ROOT (-1)
/* the game is called through ns_call (mods/nscreen.h); ns_unwinding: the game unwinds (a game
 * loaded, quit), stop */
static void flush_keys(void) { ns_call(GF_flush_keys, 0); }
static int key_pressed(void) { return ns_call(GF_key_pressed, 0); }
static int get_key(void) { return (u16)ns_call(GF_get_key, 0); }
static void mouse_hide(void) { ns_call(GF_mouse_hide, 0); }
static void mouse_show(void) { ns_call(GF_mouse_show, 0); }
static int click_record(void) { return ns_call(ROOT, 0x047c, 0x1320, 4, 6, 0, 0, 0); }     /* (the clicks so far: taken) */
static void panel_redraw(void) { ns_call(GF_party_panel_frames, 1, 0x100); ns_call(GF_party_panel_rows, 1, 0x100); }
static void frame(void) { ns_call(43, 0x150b, 0x28e6, 1, g_rw(A_CARD_WIN)); }
static void copy_picture(void) { ns_call(GF_gfx_copy_rect, 8, g_rw(A_SCREEN), 0x43, 6, 0xf5, 0xbc, g_rw(A_CARD_WIN), 7, 6); }

/* ---------------------------------------------------------------- on or off */
int cardview_active(void)
{
    return cardview_on && !plat_headless() && !config_get_int("original", 0);
}
int cardview_showing(void) { return cardview.phase != 0; }

/* ---------------------------------------------------------------- the text into pieces */
static void parse(int block)
{
    CardView *c = &cardview;
    unsigned char buf[4000];
    int n = 0;
    while (n < (int)sizeof buf - 1) { unsigned char ch = g_rb((u16)(A_MSG_BUF + n)); if (!ch) break; buf[n++] = ch; }
    buf[n] = 0;
    c->npieces = 0; c->nlines = 0; c->dropcap = 0;
    c->block = block;
    c->x = 60 + (s16)g_rw((u16)(A_BLK_X + 2 * block)) + 5;
    c->y = g_rb((u16)(A_BLK_Y + block)) + 5;
    /* the width is counted in the original's measure, whose letters are wider than the pixels
       they cover (draw_text_wrapped wraps "You gather around the comfortable fire at the" at 230
       units into 136 pixels): about 0.6 of a pixel per unit */
    c->w = (int)(((s16)g_rw((u16)(A_BLK_W + 2 * block)) - 10) * 0.6f + 0.5f);
    if (c->w < 40) c->w = 40;
    if (c->x + c->w > 316) c->w = 316 - c->x;
    c->dark = g_rw(A_TEXTCOL) == 15;
    if (getenv("DL_CARDVIEW_LOG")) fprintf(stderr, "cardview: card %.12s block %d: x %d y %d w %d (raw x %d w %d) dark %d\n", (const char *)M + LIN(G_DGROUP, 0xe3d4), block, c->x, c->y, c->w,
                                           (s16)g_rw((u16)(A_BLK_X + 2 * block)), (s16)g_rw((u16)(A_BLK_W + 2 * block)), c->dark);
    int i = 0;
    if (buf[0] > '@' && buf[0] < '[') { c->dropcap = buf[0]; i = 1; }      /* the illuminated letter (draw_text_wrapped's icon) */
    int kh = 0, linka = 0;
    CvPiece *cur = 0;                    /* the piece being filled */
    int tl = 0;                          /* its text length */
    #define NEWPIECE(o) do { if (c->npieces >= 48) { cur = 0; break; } cur = &c->piece[c->npieces++]; memset(cur, 0, sizeof *cur); cur->is_opt = (o); cur->line = -1; tl = 0; } while (0)
    #define PUT(ch) do { if (!cur) NEWPIECE(0); if (cur && tl < (int)sizeof cur->text - 1) { cur->text[tl++] = (char)(ch); cur->text[tl] = 0; } } while (0)
    for (; i < n; i++) {
        unsigned char ch = buf[i];
        if (ch == 0x01) {                                  /* a part shown only when its condition holds */
            if (!g_rb((u16)(A_CONDS + kh))) { while (i + 1 < n && buf[i + 1] != 0x13) i++; i++; }
            kh++;
            continue;
        }
        if (ch == 0x13 || ch == 0x0d) continue;
        if (ch == 0x15 || ch == 0x10 || ch == 0x16 || ch == 0x06) {
            int st = linka < 20 ? g_rb((u16)(A_OPT_ON + linka)) : 0;
            if (!st) {                                     /* switched off: the line is not shown */
                while (i + 1 < n && buf[i + 1] != '\n') i++;
                linka++;
                continue;
            }
            NEWPIECE(1);
            if (!cur) break;
            cur->opt = linka; cur->state = st;
            cur->type = ch == 0x10 ? 1 : ch == 0x16 ? 2 : ch == 0x06 ? -1 : 0;
            if (st == 1 && c->nlines < 20) cur->line = c->nlines++;
            linka++;
            continue;
        }
        if (ch == '\n') {
            if (cur && cur->is_opt) cur = 0;               /* an option ends with its line */
            else if (cur && tl) PUT('\n');
            continue;
        }
        if (ch == 0x14) {                                  /* a new paragraph (the original: 2 pixels more) */
            if (cur && !cur->is_opt) cur = 0;
            continue;
        }
        if (ch == 0x1d) {                                  /* "..." then the option's words */
            if (cur && cur->is_opt) { snprintf(cur->lead, sizeof cur->lead, "%s", cur->text); cur->text[0] = 0; tl = 0; }
            continue;
        }
        if (cur && cur->is_opt && !tl && !cur->lead[0] && ch == ' ') continue;
        PUT(ch);
    }
    #undef PUT
    #undef NEWPIECE
    /* trailing line breaks and blank paragraphs go */
    int k = 0;
    for (int j = 0; j < c->npieces; j++) {
        CvPiece *p = &c->piece[j];
        int L = (int)strlen(p->text);
        while (L && (p->text[L - 1] == '\n' || p->text[L - 1] == ' ')) p->text[--L] = 0;
        if (!p->is_opt && !L) continue;
        if (k != j) c->piece[k] = *p;
        k++;
    }
    c->npieces = k;
    c->hover = c->hover_entry = -1; c->key_line = -1;
    c->popup_piece = -1; c->nentries = 0; c->want_popup = -1;
    for (int j = 0; j < 48; j++) c->row_y0[j] = c->row_y1[j] = -1;
    c->seq++;
}

/* the option layout the rest of Darklands+ reads (D_e96e), from the pieces and their rows */
static void layout_out(void)
{
    CardView *c = &cardview;
    g_wb(A_LAYOUT, (u8)c->nlines);
    g_ww((u16)(A_LAYOUT + 4), (u16)(c->box_x0 > 0 ? (int)c->box_x0 - 60 : c->x - 60));      /* x (from the card window) */
    g_ww((u16)(A_LAYOUT + 2), (u16)(c->box_x1 > c->box_x0 ? (int)(c->box_x1 - c->box_x0) : c->w));
    for (int j = 0; j < c->npieces; j++) {
        const CvPiece *p = &c->piece[j];
        if (p->line < 0) continue;
        int y0 = c->row_y0[j] >= 0 ? (int)(c->row_y0[j] + 0.5f) : c->y + 10 * p->line;
        int y1 = c->row_y1[j] > c->row_y0[j] ? (int)(c->row_y1[j] + 0.5f) : y0 + 9;
        if (y0 < 0) y0 = 0; if (y0 > 199) y0 = 199;
        g_wb((u16)(A_LAYOUT + 6 + p->line), (u8)y0);
        g_wb((u16)(A_LAYOUT + 0x1a + p->line), (u8)(y1 - y0 > 0 ? y1 - y0 : 1));
        g_wb((u16)(A_LAYOUT + 0x42 + p->line), (u8)p->opt);
        g_ww((u16)(A_LAYOUT + 0x56 + 2 * p->line), (u16)p->type);
    }
}

const char *cardview_option_text(int opt)
{
    for (int j = 0; j < cardview.npieces; j++) if (cardview.piece[j].is_opt && cardview.piece[j].opt == opt) return cardview.piece[j].text;
    return 0;
}

/* ---------------------------------------------------------------- the picture, without text */
static void show_picture(int how)
{
    mouse_hide();
    if (how && how != 2) {                                     /* a clean page */
        ns_call(GF_gfx_fill_rect, 6, g_rw(A_CARD_WIN), 7, 6, 0xf5, 0xbc, 0xff);
        frame();
    } else if (g_rw(A_EE55)) {
        g_ww(A_EE55, 0);
        ns_call(GF_screen_hold, 0);
        ns_call(GF_set_palette, 1, A_PALETTE);
        frame(); copy_picture();
        ns_call(GF_screen_release, 0);
    } else {
        ns_call(GF_set_palette, 1, A_PALETTE);
        frame(); copy_picture();
    }
    mouse_show();
}

/* 13f2: the card again after a member's sheet or the party screen: the original reloads the
 * picture and draws the text; the text is wiped again */
static void redraw_hook(void)
{
    if (!cardview.phase) { hook_call_original(); return; }
    g_ww(A_E898, 0);                                           /* (no fade of the option list) */
    hook_call_original();
    if (unw_active) return;
    mouse_hide(); frame(); copy_picture(); mouse_show();
}

/* ---------------------------------------------------------------- waiting */
static int btn_prev;
static int left_released(int *mx, int *my)
{
    int b = g_rb(A_MOUSE_BTN) & 1;
    int r = btn_prev && !b;
    btn_prev = b;
    if (r) plat_mouse_game(mx, my);
    return r;
}

/* the loop the original runs while a card waits (menu bar, portraits, the party changed) */
static int common_step(void)
{
    ns_nested = 2;                                       /* (hidden while the bar is open or its choice runs) */
    ns_nested_since = plat_ticks_ms();
    ns_call(GF_menu_bar_run, 2, A_MENUS, 0);                   /* menu_bar_run */
    ns_nested = 0;
    if (ns_unwinding) return 0;
    if (g_rw(A_ICONS_DIRTY)) {
        g_ww(A_ICONS_DIRTY, 0);
        panel_redraw();
        mouse_hide(); frame(); copy_picture(); mouse_show();
    }
    {   int assist_test_screens(void);                         /* (test commands: a screen opened from the menu) */
        ns_nested = 1;
        int t = assist_test_screens();
        ns_nested = 0;
        if (t) { ns_call(43, 0x150b, 0x13f2, 0); return 1; }
    }
    if (g_rb(A_MOUSE_BTN) & 1) {                               /* a portrait: that member's sheet */
        int mx, my; plat_mouse_game(&mx, &my);
        if (mx < 0x3c) {
            ns_nested = 1;
            int r = ns_call(43, 0x150b, 0x13a0, 0);
            ns_nested = 0;
            if (r) { btn_prev = 0; click_record(); return 1; }
        }
    }
    return 1;
}

static void idle(void)
{
    void assist_menu_poll(void); if (cardview.phase == 2) assist_menu_poll();
    ns_call(GF_exit_if_requested, 0);                               /* exit_if_requested */
    dl_poll();
    plat_sleep_ms(3);
}

static void global_key(int key)
{
    if (key == 0x4400) {                                      /* F10: the menus (hot_keys' way; the card took the key first) */
        g_wb(0x9c26, 1); g_wb(0x9c2f, 0); g_wb(0x9c3c, 0xff); g_wb(0x9c41, 0xff);
        return;
    }
    if (ns_hot_key(key)) return;                              /* Alt-S, Alt-L ...: the game's hot keys */
    ns_nested = key >= 0x3b00 && key <= 0x4000 && !(key & 0xff);   /* F1..F6 open a screen */
    ns_call(GF_card_global_keys, 1, key);
    ns_nested = 0;
}

/* card_text (block, how): the picture, the text natively, wait for a click or Enter */
static void text_hook(void)
{
    if (!cardview_active()) { hook_call_original(); return; }
    int block = (s16)rw(SS, (u16)(SP + 4)), how = (s16)rw(SS, (u16)(SP + 6));
    ns_unwinding = 0;
    g_ww(A_COMBAT_CLK, 0);
    flush_keys();
    g_ww(A_E959, (u16)block);
    g_ww(A_E898, 0);
    show_picture(how);
    ns_call(GF_text_expand, 2, block, A_MSG_BUF);               /* text_expand */
    if (ns_unwinding) return;
    parse(block);
    cardview.how = how;
    cardview.phase = 1;
    ns_enter(NS_CARD);
    click_record(); flush_keys();
    btn_prev = g_rb(A_MOUSE_BTN) & 1;
    int key = 0;
    for (;;) {
        if (!common_step()) break;
        int mx, my;
        if (left_released(&mx, &my) && mx >= 0x3c) { key = 0x1c0d; break; }
        if (key_pressed()) {
            int k = get_key();
            if (k == 0x1c0d || k == 0x3920 || k == 0x1000) { key = k; break; }
            global_key(k);
        }
        if (ns_unwinding) break;
        idle();
        if (ns_unwinding) break;
    }
    cardview.phase = 0;
    ns_leave();
    if (ns_unwinding) return;
    if (key == 0x1000) ns_call(GF_quit_to_dos, 0);            /* quit_to_dos */
    click_record();
    flush_keys();
    if (ns_unwinding) return;
    hook_return_far(0);
}

/* card_options (block): the option list's text; the menu follows */
static void options_hook(void)
{
    if (!cardview_active()) { hook_call_original(); return; }
    int block = (s16)rw(SS, (u16)(SP + 4));
    ns_unwinding = 0;
    fixes_card_opts();                                 /* (mods/fixes.c: options some cards grey by mistake) */
    flush_keys();
    g_ww(A_COMBAT_CLK, 0);
    g_ww(A_E959, (u16)block);
    g_ww(A_E898, 0);
    show_picture(2);
    ns_call(GF_text_expand, 2, block, A_MSG_BUF);
    if (ns_unwinding) return;
    parse(block);
    cardview.how = 2;
    layout_out();
    hook_return_far(0);
}

/* the saints or items a member can call on for the option in piece j (the original's pop-up:
 * 2252 writes its lines into msg_buf and the availability into D_ea92) */
static void build_popup(int j)
{
    CardView *c = &cardview;
    const CvPiece *p = &c->piece[j];
    c->nentries = 0; c->popup_piece = j; c->popup_kind = p->type;
    /* as card_mouse: the pop-up's layout is the items' or the saints' (it carries their
       handlers, D_eb0a[slot], which the script calls for a choice from the pop-up) */
    { u16 src = p->type == 1 ? A_POP_ITEMS : A_POP_SAINTS; for (int i = 0; i < 0xf6; i++) g_wb((u16)(A_POPUP_LAY + i), g_rb((u16)(src + i))); }
    ns_call(43, 0x150b, 0x2252, 3, g_rw(A_CARD_WIN), 0, p->type);
    if (ns_unwinding) return;
    char name[24] = "";
    int k = -1;                                                /* the \x15 line: member k/4, slot k%4 */
    for (int i = 0; i < 1600; i++) {
        unsigned char ch = g_rb((u16)(A_MSG_BUF + i));
        if (!ch) break;
        if (ch == 0x81) {                                      /* "\x81<name>\xff\n": a member */
            int n = 0;
            while (n < (int)sizeof name - 1) { unsigned char d = g_rb((u16)(A_MSG_BUF + i + 1 + n)); if (!d || d == 0xff || d == '\n') break; name[n++] = (char)d; }
            name[n] = 0;
            i += n;
            continue;
        }
        if (ch != 0x15) continue;
        k++;
        int code = 0x80, n = 0; char t[60];
        int q = i + 1;
        if (g_rb((u16)(A_MSG_BUF + q)) == ' ') q++;
        unsigned char cc = g_rb((u16)(A_MSG_BUF + q));
        if (cc >= 0x80 && cc <= 0x8f) { code = cc == 0x8f ? 0x80 : cc; q++; }
        while (n < (int)sizeof t - 1) { unsigned char d = g_rb((u16)(A_MSG_BUF + q + n)); if (!d || d == '\n') break; t[n++] = (char)d; }
        t[n] = 0;
        int en = k < 20 ? g_rb((u16)(A_EA92 + k)) : 0;           /* = the box layout's en[]: 0 hidden, 1 choosable, 2 shown */
        if (!n || !en || c->nentries >= CV_MAXPOP) continue;
        CvEntry *e = &c->entry[c->nentries++];
        memset(e, 0, sizeof *e);
        e->member = k / 4; e->slot = k % 4; e->code = code;
        e->on = en == 1;
        e->id = (s16)g_rw((u16)((p->type == 1 ? A_CARD_ITEMS : A_CARD_SAINTS) + 2 * e->slot));
        snprintf(e->text, sizeof e->text, "%s", t);
        snprintf(e->who, sizeof e->who, "%s", name);
        name[0] = 0;
    }
    for (int i = 0; i < CV_MAXPOP; i++) c->entry_y0[i] = c->entry_y1[i] = -1;
    if (getenv("DL_CARDVIEW_LOG")) { fprintf(stderr, "cardview: pop-up for option %d (kind %d): %d entries\n", p->opt, p->type, c->nentries);
        for (int i = 0; i < c->nentries; i++) fprintf(stderr, "  member %d slot %d on %d code %02x %s\n", c->entry[i].member, c->entry[i].slot, c->entry[i].on, c->entry[i].code, c->entry[i].text); }
}

/* the piece / pop-up entry under game pixel (mx, my) */
/* (the game's pointer is a hand whose point is a few pixels below the spot it reports) */
#define HAND_DY 2
static int piece_at(int mx, int my)
{
    const CardView *c = &cardview;
    my += HAND_DY;
    if (mx < c->box_x0 - 2 || mx > c->box_x1 + 2) return -1;
    for (int j = 0; j < c->npieces; j++) if (c->piece[j].is_opt && c->row_y1[j] > c->row_y0[j] && my >= c->row_y0[j] && my < c->row_y1[j]) return j;
    return -1;
}
static int entry_at(int mx, int my)
{
    const CardView *c = &cardview;
    my += HAND_DY;
    if (c->popup_piece < 0 || mx < c->box_x0 - 2 || mx > c->box_x1 + 2) return -1;
    for (int i = 0; i < c->nentries; i++) if (c->entry_y1[i] > c->entry_y0[i] && my >= c->entry_y0[i] && my < c->entry_y1[i]) return i;
    return -1;
}

/* card_menu: the option chosen (the script calls opt_fn[it]); from the pop-up, menu_member and
 * menu_slot say whose saint or item, and menu_popup is 1 */
int cardview_menu(void)
{
    CardView *c = &cardview;
    ns_unwinding = 0;
    g_ww(A_COMBAT_CLK, 0);
    if (g_rb(A_9C4C)) click_record();
    flush_keys();
    g_wb(A_EE41, 0); g_wb(A_EE42, 0);
    g_ww(A_MENU_POPUP, 0); g_ww(A_8DDE, 0);
    g_wb(A_HILITE, 0xff);
    if (c->npieces == 0) parse((s16)g_rw(A_E959));     /* (a menu without card_options before it) */
    c->phase = 2;
    ns_enter(NS_CARD);
    btn_prev = g_rb(A_MOUSE_BTN) & 1;
    int sel = -1;
    for (;;) {
        if (!common_step()) break;
        layout_out();
        if (c->want_popup >= 0) { int j = c->want_popup; c->want_popup = -1; if (c->popup_piece == j) { c->popup_piece = -1; c->nentries = 0; } else build_popup(j); if (ns_unwinding) break; }
        int mx, my;
        plat_mouse_game(&mx, &my);
        { static int lx = -1, ly = -1; if (mx != lx || my != ly) { if (lx >= 0) c->key_line = -1; lx = mx; ly = my; } }   /* the mouse moved: it leads again */
        c->hover = c->key_line >= 0 ? -1 : piece_at(mx, my);
        c->hover_entry = entry_at(mx, my);
        if (left_released(&mx, &my)) {
            int e = entry_at(mx, my), j = piece_at(mx, my);
            if (e >= 0 && c->entry[e].on) {
                g_ww(A_MENU_MEMBER, (u16)c->entry[e].member); g_ww(A_MENU_SLOT, (u16)c->entry[e].slot);
                g_ww(A_MENU_POPUP, 1);
                g_wb(A_EE42, 1);                               /* (as the original leaves it after a choice from its box) */
                sel = c->piece[c->popup_piece].opt;
                break;
            }
            if (j >= 0 && c->piece[j].state == 1) {
                if (c->piece[j].type == 1 || c->piece[j].type == 2) c->want_popup = j;   /* list the saints / items */
                else { sel = c->piece[j].opt; break; }
            }
        }
        if (key_pressed()) {
            int k = get_key();
            if (k == 0x1000) ns_call(GF_quit_to_dos, 0);      /* quit_to_dos (asks) */
            else if (k == 0x4800 || k == 0x5000) {             /* Up / Down through the choosable options */
                int n = c->nlines;
                if (n > 0) c->key_line = c->key_line < 0 ? (k == 0x4800 ? n - 1 : 0) : (c->key_line + (k == 0x4800 ? n - 1 : 1)) % n;
            } else if ((k == 0x1c0d || k == 0x3920) && c->key_line >= 0) {
                for (int j = 0; j < c->npieces; j++) if (c->piece[j].line == c->key_line) {
                    if (c->piece[j].type == 1 || c->piece[j].type == 2) c->want_popup = j;
                    else sel = c->piece[j].opt;
                }
                if (sel >= 0) break;
            } else global_key(k);
        }
        if (ns_unwinding) break;
        idle();
        if (ns_unwinding) break;
    }
    c->phase = 0;
    ns_leave();
    c->popup_piece = -1;
    if (ns_unwinding) return -1;
    g_ww(A_8DDE, 1);
    for (int j = 0; j < c->npieces; j++) if (c->piece[j].opt == sel && c->piece[j].line >= 0) g_wb(A_HILITE, (u8)c->piece[j].line);
    click_record();
    ns_call(ROOT, 0x047c, 0x059e, 1, 0);
    if (getenv("DL_CARDVIEW_LOG")) fprintf(stderr, "cardview: chose option %d (pop-up %d member %d slot %d)\n", sel, g_rw(A_MENU_POPUP), g_rw(A_MENU_MEMBER), g_rw(A_MENU_SLOT));
    return sel;
}

int cardview_unwinding(void) { return ns_unwinding; }

/* (tests) what is on show: the pieces, their rows (game pixels) and the pop-up */
void cardview_dump(void)
{
    const CardView *c = &cardview;
    fprintf(stderr, "cv phase %d block %d pieces %d lines %d box %.0f..%.0f\n", c->phase, c->block, c->npieces, c->nlines, c->box_x0, c->box_x1);
    for (int j = 0; j < c->npieces; j++) {
        const CvPiece *p = &c->piece[j];
        if (p->is_opt) fprintf(stderr, "cv opt %d line %d type %d state %d rows %.1f..%.1f: %s%s\n", p->opt, p->line, p->type, p->state, c->row_y0[j], c->row_y1[j], p->lead, p->text);
        else fprintf(stderr, "cv text: %.70s\n", p->text);
    }
    for (int i = 0; i < c->nentries; i++) fprintf(stderr, "cv entry %d member %d slot %d on %d rows %.1f..%.1f: %s\n", i, c->entry[i].member, c->entry[i].slot, c->entry[i].on, c->entry_y0[i], c->entry_y1[i], c->entry[i].text);
}

/* the game's own questions and messages ("Quit to DOS? y/n", hot keys' messages, a card
 * script's message box) are drawn into the picture: the native card steps aside meanwhile */
int cardview_dialog;
static void dialog_hook(void) { cardview_dialog++; hook_call_original(); cardview_dialog--; }

static void hooks(void)
{
    hook_install(GF_ask_yes_no, dialog_hook);          /* ask_yes_no */
    hook_install(GF_show_message, dialog_hook);          /* show_message */
    hook_install(GF_message_box, dialog_hook);            /* message_box */
    if (cardview_on) {
        hook_install(GF_card_text, text_hook);
        hook_install(GF_card_options, options_hook);
        hook_install(43, 0x150b, 0x13f2, redraw_hook);
        void assist_watch_cards(void); assist_watch_cards();    /* card_menu's hook (mods/assist.c) calls cardview_menu */
    } else {
        hook_remove(GF_card_text, text_hook);
        hook_remove(GF_card_options, options_hook);
        hook_remove(43, 0x150b, 0x13f2, redraw_hook);
    }
}

void cardview_set(int on)
{
    cardview_on = on;
    config_set_int("display.native_cards", on);
    hooks();
}

void cardview_init(void)
{
    cardview_on = config_get_int("display.native_cards", 1);
    hooks();
}
