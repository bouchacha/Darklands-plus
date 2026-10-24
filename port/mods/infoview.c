/* infoview.c - the party information screen as a native page (display.native_info; the view is
 * ui/infoview.cpp).
 *
 * The original (party_info, s034 1550:0006, pics\ptystats.pic) shows the party's fame and the
 * word for it, the time and date, where the party is, the purse, the notes and the Philosopher's
 * Stone, the reputation here, and a small map: the town nearest the pointer is named with its
 * size and the party's reputation there. Here the same, read the same way (the fame words at far
 * 290e:1f7b, the time-of-day names 20c3 through hour_band, the months 1d43, the reputation words
 * 2183 through loc_query(loc, 1), the directions 20e3), with the reputation in every city at
 * once. It shows and changes nothing else; leaving is the original's (party_icons_dirty, the
 * pointer's events flushed). */
#include <stdio.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "gamemodel.h"
#include "nscreen.h"
void plat_hold(int ms);
#include "infoview.h"

void plat_sleep_ms(int ms);
void dl_poll(void);

InfoModel info_model;
static int iv_on = 1;
#define T290E ((u16)(LOADSEG + 0x290e))
#define VECSEG ((u16)(LOADSEG + 0x9c0))

int infoview_is_on(void) { return iv_on; }
void infoview_set(int on) { iv_on = on; config_set_int("display.native_info", on); }

static void far_str(u16 toff, int i, char *out, int n)    /* entry i of a far table (290e) of far strings */
{
    u16 o = rw(T290E, (u16)(toff + 4 * i)), s = rw(T290E, (u16)(toff + 4 * i + 2));
    out[0] = 0;
    if (s) g_str(s, o, out, n);
}

static void build(void)
{
    InfoModel *m = &info_model;
    m->fame = (s16)g_rw(GV_party_fame);
    int lv = m->fame / 50; if (lv > 10) lv = 10; if (lv < 0) lv = 0;
    far_str(0x1f7b, lv, m->fame_word, sizeof m->fame_word);
    m->hour = (s16)g_rw(GV_cur_hour); m->day = (s16)g_rw(GV_cur_day); m->month = (s16)g_rw(GV_cur_month); m->year = (s16)g_rw(GV_cur_year);
    gm_hour_name(m->hour, m->bell, sizeof m->bell);                  /* (the game's bell names, its "Latins" put right: mods/gamemodel.c) */
    if (m->month >= 0 && m->month < 12) far_str(0x1d43, m->month, m->month_name, sizeof m->month_name); else m->month_name[0] = 0;
    int cur = (s16)g_rw(GV_cur_loc);
    m->in_city = cur >= 0 && cur < IV_CITIES;
    if (m->in_city) {
        u32 r = g_location(cur); m->here = cur;
        m->where[0] = 0; if (r) g_str((u16)(r >> 16), (u16)(r + 0x26), m->where, sizeof m->where);
    } else {
        int px = (s16)g_rw(GV_party_x), py = (s16)g_rw(GV_party_y);
        int nc = ns_call_ptr(VECSEG, 0x1fa9, 2, px, py);          /* nearest_city */
        u32 r = g_location(nc);
        char dir[24] = "", name[24] = "";
        if (r) {
            int d = ns_call_ptr(VECSEG, 0x1fbd, 4, (s16)rw((u16)(r >> 16), (u16)(r + 4)), (s16)rw((u16)(r >> 16), (u16)(r + 6)), px, py);
            far_str(0x20e3, d, dir, sizeof dir);
            g_str((u16)(r >> 16), (u16)(r + 0x26), name, sizeof name);
        }
        m->here = nc >= 0 && nc < IV_CITIES ? nc : -1;
        snprintf(m->where, sizeof m->where, "%s of %s", dir, name);
    }
    for (int i = 0; i < 3; i++) m->money[i] = (s16)g_rw((u16)(GV_party_money + 2 * i));
    { u16 lo = (u16)ns_call(GF_money_pfennigs, 1, GV_party_money); m->total_pf = (long)(((u32)(u16)ns_call_dx() << 16) | lo); }   /* (the game's own sum) */
    m->notes = (s16)g_rw(GV_party_notes);
    m->phstone = (s16)g_rw(GV_philosophers_stone);
    for (int i = 0; i < IV_CITIES; i++) {
        u32 r = g_location(i);
        m->city[i].rep = r ? (s16)rw((u16)(r >> 16), (u16)(r + 0x12)) : 0;
        m->city[i].size = r ? rb((u16)(r >> 16), (u16)(r + 0x11)) : 0;
        m->city[i].band = ns_call(GF_loc_query, 2, i, 1);
        far_str(0x2183, m->city[i].band, m->city[i].band_word, sizeof m->city[i].band_word);
    }
    m->seq++;
}

static void screen_hook(void)                 /* party_info (), s034 1550:0006 */
{
    if (!iv_on || !ns_active()) { hook_call_original(); return; }
    plat_hold(600);                              /* (the old screen drawn while it is set up: not shown) */
    InfoModel *m = &info_model;
    ns_unwinding = 0;
    build();
    if (ns_unwinding) return;
    m->open = 1;
    fprintf(stderr, "party info: open (fame %d, %s)\n", m->fame, m->where);
    ns_enter(NS_INFO);
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
            if (k == 0x011b || k == 0x1c0d || k == 0x3920) done = 1;    /* Esc, Enter, Space */
            else if (k == 0x4800 || k == 0x5000) ns_in.wheel += k == 0x4800 ? 1 : -1;   /* (the mouse wheel comes as Up / Down: the towns' list) */
            else if (k == 0x4900 || k == 0x5100) ns_in.wheel += k == 0x4900 ? 4 : -4;
        }
        NsAction a;
        while (ns_pop(&a)) if (a.type == IV_CLOSE) done = 1;
    }
    m->open = 0;
    ns_leave();
    fprintf(stderr, "party info: closed\n");
    if (ns_unwinding) return;
    g_ww(GV_party_icons_dirty, 1);                                  /* the original's leaving */
    ns_call(-1, 0x047c, 0x1320, 4, 6, 0, 0, 0);
    hook_return_far(0);
}

void infoview_init(void)
{
    iv_on = config_get_int("display.native_info", 1);
    hook_install(GF_party_info_screen, screen_hook);
}
