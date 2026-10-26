/* testcmds.c - the test commands that look at or change the game's state, for the scripted
 * sessions (port/tools/nctl.py "ucmd NAME:ARGS", release_check.py, monkey.py): a battle at the next
 * card, hits, items, saves, the native screens' actions, dumps of what the game holds ...
 * platform_sdl.c keeps those that act as the system would (keys, clicks, the wheel, a controller)
 * and hands every other step here. Not part of play: nothing calls these but a script. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "hle.h"
#include "../ui/ui.h"
#include "../mods/hdart.h"
#include "../mods/hdtext.h"
#include "game.h"
#include "gamevars.h"
#include "../mods/losaim.h"
#include "config.h"

void plat_mouse_game(int *x, int *y);
u32 plat_ticks_ms(void);

/* ualcrisk: many failed brews at this place, counting the accidents they bring (mods/fixes.c, alchrisk) */
#include "hooks.h"
static void quiet_message(void) { hook_return_far(0); }
#define ev_far g_event
static void alc_risk_test(int risk, int n)
{
    int boom = 0, stink = 0, none = 0, loc = (s16)g_rw(GV_cur_loc);
    hook_install(GF_show_message, quiet_message);
    for (int k = 0; k < n; k++) {
        u16 a[10] = { 0, (u16)loc, 1, 0x1d, 0, 0, 1, 0, 0, 0 };     /* member 0's brew of formula 0, chance 0 */
        game_call_far(GF_event_new, 10, a);
        int i = (s16)AX; u32 e = i >= 0 ? ev_far(i) : 0;
        if (!e) { fprintf(stderr, "alcrisk: no event\n"); break; }
        ww((u16)(e >> 16), (u16)((u16)e + 0x2a), 0); ww((u16)(e >> 16), (u16)((u16)e + 0x2c), (u16)risk);
        g_ww(GV_last_event, (u16)i);
        u16 b[1] = { (u16)i };
        game_call_far(29, 0x0e76, 0x3fb2, 1, b);
        int j = (s16)g_rw(GV_last_event); u32 f = j != i && j >= 0 ? ev_far(j) : 0;
        if (f && (s16)rw((u16)(f >> 16), (u16)((u16)f + 0x22)) == 0x51) {
            if ((s16)rw((u16)(f >> 16), (u16)((u16)f + 0x1e))) boom++; else stink++;
            u16 c[1] = { (u16)j }; game_call_far(GF_event_free, 1, c);
        } else none++;
        game_call_far(GF_event_free, 1, b);
    }
    hook_remove(GF_show_message, quiet_message);
    fprintf(stderr, "alcrisk %d at %d: %d explosions, %d stinks, %d nothing (of %d failures)\n", risk, loc, boom, stink, none, n);
}

int testcmd_run(const char *cmd, const char *arg)
{
    int x = 0, y = 0;
    sscanf(arg, "%d,%d", &x, &y);
    (void)x; (void)y;
    if (0) {}
        else if (!strcmp(cmd, "ugive")) { int a = 0, b = 0, c = 0; sscanf(arg, "%d,%d,%d", &a, &b, &c); void ui_give(int, int, int); ui_give(a, b, c); }
        else if (!strcmp(cmd, "umods")) { void datamods_scan(void); int encounters_count(void); datamods_scan(); fprintf(stderr, "mods scanned: %d new encounters\n", encounters_count()); }   /* test: "Look for mods again" (F12 > Mods) */
        else if (!strcmp(cmd, "upreset")) { void preset_apply(int); preset_apply(atoi(arg)); void config_save(void); config_save(); fprintf(stderr, "preset %d (saved)\n", atoi(arg)); }   /* test: a preset (0 Buggy Purist, 1 Purist, 2 Purist + interface ...) */
        else if (!strcmp(cmd, "uset")) { char k[64]; int v = 0; if (sscanf(arg, "%63[^=]=%d", k, &v) == 2) { config_set_int(k, v); fprintf(stderr, "set %s = %d\n", k, v); } }   /* test: a setting */
        else if (!strcmp(cmd, "uenemyinv")) {          /* test: enemy i's items (far 2e38:0dbe [i-5][16], count at +7e) */
            int i = atoi(arg); if (i < 5 || i > 12) i = 5;
            int n = (s8)g_rb(CBF(i, CB_NITEMS));
            fprintf(stderr, "enemyinv %d (%d):", i, n);
            for (int k = 0; k < 16; k++) { u16 o = (u16)(0x0dbe + (i - 5) * 96 + k * 6); fprintf(stderr, " [%d] %d/%d q%d x%d", k, rw(G_FAR2E38, o), rb(G_FAR2E38, (u16)(o + 2)), rb(G_FAR2E38, (u16)(o + 3)), rb(G_FAR2E38, (u16)(o + 4))); }
            fprintf(stderr, "\n");
        }
        else if (!strcmp(cmd, "upicpng")) { int gamepics_write_png(const char *, const char *, int, const char *); char nm[64], path[200], pp[64] = "", ppd[80] = ""; int sc = 4;   /* test: PICS\NAME.PIC as a PNG in today's colours */
            if (sscanf(arg, "%63[^,],%d,%63s", nm, &sc, pp) >= 1) { char dos[80]; snprintf(dos, sizeof dos, "PICS\\%s.PIC", nm); snprintf(path, sizeof path, "pic_%s.png", nm);
                if (pp[0]) snprintf(ppd, sizeof ppd, "PICS\\%s.PIC", pp);
                fprintf(stderr, "upicpng %s: %d\n", path, gamepics_write_png(dos, path, sc, ppd)); } }
        else if (!strcmp(cmd, "uiconsheet")) { void worldmap_icon_sheet(void); worldmap_icon_sheet(); }   /* test: the place pictures of each set, large */
        else if (!strcmp(cmd, "uornsheet")) { void orn_test_sheet(void); orn_test_sheet(); }   /* test: the frame and button styles side by side */
        else if (!strcmp(cmd, "unichesheet")) { void niche_test_sheet(void); niche_test_sheet(); }   /* test: the niche's corner styles side by side */
        else if (!strcmp(cmd, "uclocksheet")) { void clock_test_sheet(void); clock_test_sheet(); }   /* test: the clocks large, side by side */
        else if (!strcmp(cmd, "ulcd")) { void ui_mt32_lcd(const char *); ui_mt32_lcd(arg); }   /* test: the MT-32 display shows the text */
        else if (!strcmp(cmd, "upray")) { int m = 0, st = 0; sscanf(arg, "%d,%d", &m, &st); void sheetview_test_pray(int, int); sheetview_test_pray(m, st); }   /* test: member m prays to saint st (the prayer screen) */
        else if (!strcmp(cmd, "ulos")) {               /* test: the furniture on the battle floor, and the first id that blocks sight */
            int hist[16] = { 0 };
            for (int k = 0; k < 40 * 40; k++) { u32 v = rw(G_FAR20A5, (u16)(4 * k)) | (u32)rw(G_FAR20A5, (u16)(4 * k + 2)) << 16; hist[(v >> 4) & 15]++; }
            fprintf(stderr, "los: first blocking furniture %d; furniture by id:", (s8)g_rb(0xc1cf));
            for (int f = 1; f < 16; f++) if (hist[f]) fprintf(stderr, " %d:%d", f, hist[f]);
            fprintf(stderr, "\n");
        }
        else if (!strcmp(cmd, "uhsect")) { void settings_helpers_jump(const char *); settings_helpers_jump(arg); }   /* test: the Game page at the section named */
        else if (!strcmp(cmd, "udsect")) { void settings_display_jump(int); settings_display_jump(atoi(arg)); }   /* test: the Display page at section N */
        else if (!strcmp(cmd, "ulosall")) {            /* test: from every member to every foe, the game's line test against mods/losaim.c's */
            typedef LosBox B;
            int ns_call(int, u16, u16, int, ...); int agree = 0, differ = 0, blocked = 0;
            for (int w = 0; w < 5; w++) for (int f = 5; f < 13; f++) {
                if (g_rb(CBF(w, CB_STATUS)) != 1 || g_rb(CBF(f, CB_STATUS)) != 1) continue;
                for (int md = 0x80; md <= 0x100; md += 0x80) {
                    B bx[24]; int n = los_trace(w, f, md, bx, 24);
                    u16 l[4] = { g_rw(GV_los_x0), g_rw(GV_los_y0), g_rw(GV_los_x1), g_rw(GV_los_y1) };
                    g_ww(GV_los_x0, g_rw(CBF(w, CB_X))); g_ww(GV_los_y0, g_rw(CBF(w, CB_Y)));
                    g_ww(GV_los_x1, g_rw(CBF(f, CB_X))); g_ww(GV_los_y1, g_rw(CBF(f, CB_Y)));
                    int bl = ns_call(GF_line_blocked, 3, w, f, md) != 0;
                    g_ww(GV_los_x0, l[0]); g_ww(GV_los_y0, l[1]); g_ww(GV_los_x1, l[2]); g_ww(GV_los_y1, l[3]);
                    if (bl == (n > 0)) agree++; else { differ++; fprintf(stderr, "losall: differ %d->%d mode %#x game %d boxes %d:", w, f, md, bl, n); for (int j = 0; j < n; j++) fprintf(stderr, " k%d id%d %d,%d-%d,%d", bx[j].kind, bx[j].id, bx[j].x0, bx[j].y0, bx[j].x1, bx[j].y1); fprintf(stderr, " | %d,%d -> %d,%d h59 %d\n", (s16)g_rw(CBF(w, CB_X)), (s16)g_rw(CBF(w, CB_Y)), (s16)g_rw(CBF(f, CB_X)), (s16)g_rw(CBF(f, CB_Y)), (s8)g_rb((u16)(0x9c55 + w * 0x80 + 0x59))); }
                    blocked += bl;
                }
            }
            fprintf(stderr, "losall: %d agree (%d blocked), %d differ\n", agree, blocked, differ);
        }
        else if (!strcmp(cmd, "ushoot")) { int i = 0, t = 5; sscanf(arg, "%d,%d", &i, &t); void orders_test_shoot(int, int); orders_test_shoot(i, t); }   /* test: member i shoots at t (as ordered) */
        else if (!strcmp(cmd, "uskysheet")) { void banner_sky_sheet(int); banner_sky_sheet(atoi(arg)); }   /* test: the travel map's sky box at every second hour */
        else if (!strcmp(cmd, "usndprev")) { char d[16] = ""; int w = 1; sscanf(arg, "%15[^,],%d", d, &w); void soundpreview_start(const char *, int); soundpreview_start(d, w); }   /* test: hear device d (mt32, fm ...) play music (0) or the battle sounds (1) */
        else if (!strcmp(cmd, "uiconstyles")) { void ui_icon_styles_sheet(int); ui_icon_styles_sheet(atoi(arg)); }   /* test: the pictures of the skills in each look */
        else if (!strcmp(cmd, "uaim")) {               /* test: who shoots, and the aim under the pointer */
            extern int orders_member_mode(int m); extern int orders_aim(int *who, int *foe);
            for (int m = 0; m < 5; m++) fprintf(stderr, "aim: member %d mode %d weapon %d order %#x\n", m, orders_member_mode(m), (s8)g_rb(CBF(m, CB_MISSILE)), g_rw(CBF(m, CB_ORDER)));
            int w = -1, f = -1, md = orders_aim(&w, &f);
            fprintf(stderr, "aim: %#x member %d foe %d selected %d\n", md, w, f, (s8)g_rb(GV_selected_member));
            if (md) {
                LosBox bx[24]; int n = los_trace(w, f, md, bx, 24);
                for (int j = 0; j < n; j++) fprintf(stderr, "aim: box kind %d id %d %d,%d-%d,%d\n", bx[j].kind, bx[j].id, bx[j].x0, bx[j].y0, bx[j].x1, bx[j].y1);
                u16 l[4] = { g_rw(GV_los_x0), g_rw(GV_los_y0), g_rw(GV_los_x1), g_rw(GV_los_y1) };
                g_ww(GV_los_x0, g_rw(CBF(w, CB_X))); g_ww(GV_los_y0, g_rw(CBF(w, CB_Y)));
                g_ww(GV_los_x1, g_rw(CBF(f, CB_X))); g_ww(GV_los_y1, g_rw(CBF(f, CB_Y)));
                int ns_call(int, u16, u16, int, ...); int bl = ns_call(GF_line_blocked, 3, w, f, md);
                g_ww(GV_los_x0, l[0]); g_ww(GV_los_y0, l[1]); g_ww(GV_los_x1, l[2]); g_ww(GV_los_y1, l[3]);
                fprintf(stderr, "aim: game says %d, boxes %d\n", bl, n);
            }
        }
        else if (!strcmp(cmd, "uscr")) {               /* test: each combatant's map and screen position, and the view's origin */
            fprintf(stderr, "scr: origin %d,%d map origin %d,%d c1cf %d\n", (s8)g_rb(0xc866), (s8)g_rb(0xc886), (s16)g_rw(0xc26c), (s16)g_rw(0xc293), (s8)g_rb(0xc1cf));
            for (int i = 0; i < 13; i++) { u16 b = (u16)(0x9c55 + i * 0x80); if (g_rb((u16)(b + 0x14)) != 1) continue;
                fprintf(stderr, "scr: %d map %d,%d screen %d,%d foot %d\n", i, (s16)g_rw((u16)(b + 4)), (s16)g_rw((u16)(b + 6)), (s16)g_rw(b), (s16)g_rw((u16)(b + 2)), (s16)g_rw((u16)(0x8650 + 2 * i))); }
        }
        else if (!strcmp(cmd, "urec")) { extern int plat_rec_frames; plat_rec_frames = atoi(arg); }   /* test: save the next N frames (DL_REC=dir) */
        else if (!strcmp(cmd, "uwalk")) { void pathfix_test_walk(int, int, int); int i = 0, x = 0, y = 0; sscanf(arg, "%d,%d,%d", &i, &x, &y); pathfix_test_walk(i, x, y); }   /* test: combatant i walks to x, y (battle map pixels) */
        else if (!strcmp(cmd, "upath")) { void pathfix_set(int, int); int pp = 1, pe = 0; sscanf(arg, "%d,%d", &pp, &pe); pathfix_set(pp, pe); fprintf(stderr, "pathfix party %d enemies %d\n", pp, pe); }   /* test: better pathfinding (party, enemies) */
        else if (!strcmp(cmd, "ubells")) { for (int b = 0; b < 8; b++) { u16 tb = (u16)(LOADSEG + 0x290e), o = rw(tb, (u16)(0x20c3 + 4 * b)), sg = rw(tb, (u16)(0x20c3 + 4 * b + 2)); char t[24] = ""; if (sg) g_str(sg, o, t, sizeof t); fprintf(stderr, "bell %d: %s\n", b, t); } }   /* test: the game's names of the hours */
        else if (!strcmp(cmd, "uaging")) { void aging_month(int); aging_month(atoi(arg) > 0 ? atoi(arg) : 1); }   /* test: months of aging at once */
        else if (!strcmp(cmd, "ufigpick")) { int figpick_open(int, int); figpick_open(atoi(arg), 0); }   /* test: the figure picker for member m */
        else if (!strcmp(cmd, "ucity")) { unsigned of = 0; int v = -999; sscanf(arg, "%x,%d", &of, &v); u16 o = g_rw(GV_city_rec), sg = g_rw((u16)(GV_city_rec + 2));
            fprintf(stderr, "city +%x: %d", of, (s16)rw(sg, (u16)(o + of))); if (v != -999) { ww(sg, (u16)(o + of), (u16)v); fprintf(stderr, " -> %d", v); } fprintf(stderr, "\n"); }   /* test: a word of the current city's record */
        else if (!strcmp(cmd, "upan")) { int h = 0, v = 0; sscanf(arg, "%d,%d", &h, &v); void battlescroll_pan(int, int); battlescroll_pan(h, v); }   /* test: the battle view moved (steps across, down) */
        else if (!strcmp(cmd, "uskill")) { int m = 0, k = 0, v = 0; sscanf(arg, "%d,%d,%d", &m, &k, &v); if (m >= 0 && m < 5 && k >= 0 && k < 19) g_wb((u16)(G_MEMBER(m) + G_M_SKILLS + k), (u8)v); }   /* test: member m's skill k */
        else if (!strcmp(cmd, "uage")) { int m = 0, y = 25; sscanf(arg, "%d,%d", &m, &y); if (m >= 0 && m < 5) g_ww((u16)(G_MEMBER(m) - 2), (u16)y); }   /* test: member m's age */
        else if (!strcmp(cmd, "ualert")) { void ui_alert_dismiss(void); ui_alert_dismiss(); }   /* test: the window of news clicked through */
        else if (!strcmp(cmd, "umaxattr")) { int m = 0, a = 0, v = 1; sscanf(arg, "%d,%d,%d", &m, &a, &v); if (m >= 0 && m < 5 && a >= 0 && a < 7) g_wb((u16)(G_MEMBER(m) + G_M_ATTR_MAX + a), (u8)v); }   /* test: an attribute's maximum */
        else if (!strcmp(cmd, "ureload")) { int m = 0, t = 100; sscanf(arg, "%d,%d", &m, &t); u16 now = g_rw(GV_combat_clock); g_wb((u16)(GV_cbt_action + m), 9); g_ww((u16)(0xd948 + 2 * m), (u16)(now + 10 - t / 3)); g_ww((u16)(0xdabd + 2 * m), (u16)(now + t / 3)); g_ww((u16)(0xdaa3 + 2 * m), (u16)(now + t)); }   /* test: member m reloading (a third done) */
        else if (!strcmp(cmd, "uhall")) { void halloffame_open(void); int halloffame_record(void); if (arg[0] == 'r') fprintf(stderr, "hall record: %d\n", halloffame_record()); else halloffame_open(); }   /* test */
        else if (!strcmp(cmd, "ulevelmap")) { void levelmap_toggle(void); levelmap_toggle(); }   /* test: Tab in a battle */
        else if (!strcmp(cmd, "artexport")) { char m[400]; hdart_export(m, sizeof m); fprintf(stderr, "%s\n", m); }
        else if (!strcmp(cmd, "artrescan")) hdart_rescan();
        else if (!strcmp(cmd, "ufight")) {              /* test: a battle (type, seed[, 3 x (enemy type, quality, number)]) at the next card */
            extern int ufight_pending, ufight_seed, ufight_groups[9]; int g[9] = { 3, 2, 5, 0, 1, 1, -1, 0, 0 };
            ufight_seed = -1;
            sscanf(arg, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &ufight_pending, &ufight_seed, &g[0], &g[1], &g[2], &g[3], &g[4], &g[5], &g[6], &g[7], &g[8]);
            for (int k = 0; k < 9; k++) ufight_groups[k] = g[k];
            if (!ufight_pending) ufight_pending = 10;
        }
        else if (!strcmp(cmd, "uthrowsim")) { extern int loot_sim_m, loot_sim_n; sscanf(arg, "%d,%d", &loot_sim_m, &loot_sim_n); }   /* test: a throw, at the next loot screen */
        else if (!strcmp(cmd, "upeek")) {               /* debug: words at link-time seg:off (+LOADSEG) */
            unsigned sg = 0, of = 0, n = 8; sscanf(arg, "%x,%x,%u", &sg, &of, &n);
            fprintf(stderr, "peek %04x:%04x:", sg, of);
            for (unsigned k = 0; k < n; k++) fprintf(stderr, " %04x", rw((u16)(LOADSEG + sg), (u16)(of + 2 * k)));
            fprintf(stderr, "\n");
        }
        else if (!strcmp(cmd, "upotion")) {             /* test: member m drinks potion code of quality q (use_potion, s031 1462:43b2) */
            int m = 0, code = 0x69, q = 30; sscanf(arg, "%d,%x,%d", &m, &code, &q);
            int n = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS));
            if (m >= 0 && m < 5 && n < 64) {
                u16 o = G_ITEM(m, n);
                ww(G_FAR2E38, o, (u16)code); wb(G_FAR2E38, (u16)(o + 2), 0); wb(G_FAR2E38, (u16)(o + 3), (u8)q); wb(G_FAR2E38, (u16)(o + 4), 1); wb(G_FAR2E38, (u16)(o + 5), 0);
                g_wb((u16)(G_MEMBER(m) + G_M_NITEMS), (u8)(n + 1));
                u16 a[2] = { (u16)m, (u16)n };
                game_call_far(31, 0x1462, 0x43b2, 2, a);
                g_wb((u16)(G_MEMBER(m) + G_M_NITEMS), (u8)n);
                fprintf(stderr, "potion %x q%d for %d: bonus byte %02x, armor byte %02x, fire %d\n", code, q, m, g_rb((u16)(G_MEMBER(m) + 0x34)), g_rb((u16)(G_MEMBER(m) + 0x41)), (s8)g_rb((u16)(G_MEMBER(m) + 0x10)));
            }
        }
        else if (!strcmp(cmd, "uicongrid")) { void ui_icon_grid_toggle(void); ui_icon_grid_toggle(); }   /* test: the potions' and ingredients' pictures */
        else if (!strcmp(cmd, "usainttab")) {           /* test: each saint's table entry t[0..6] (day, class, min x, max, virtue, base, t6), through saint_fns */
            u16 tseg = (u16)(LOADSEG + 0x290e), vseg = (u16)(LOADSEG + 0x09c0);
            void *snap = eval_snapshot();
            for (int t = 0; t < 160; t++) {
                u16 off = rw(tseg, (u16)(0x2937 + 4 * t)), seg = rw(tseg, (u16)(0x2937 + 4 * t + 2));
                if (seg != vseg || off < 0x1000) break;
                fprintf(stderr, "sainttab %d %04x:", t, off);
                for (int w = 0; w < 7; w++) { u16 a[3] = { 0, 0, (u16)w }; u32 r = game_call_ptr(seg, off, 3, a, 0); eval_restore(snap); fprintf(stderr, " %d", (s16)r); }
                fprintf(stderr, "\n");
            }
            eval_free(snap);
        }
        else if (!strcmp(cmd, "uchronfill")) { void journal_test_chronicle(int); journal_test_chronicle(atoi(arg)); }   /* test: n made-up texts in the chronicle (memory only) */
        else if (!strcmp(cmd, "usubtab")) { extern int ui_want_subtab; ui_want_subtab = atoi(arg); }   /* test: the sub-tab to open next (the journal's: 7 the chronicle) */
        else if (!strcmp(cmd, "usaintdo")) {            /* test: saint s's help for member m (saint_fns[s](m, 50, 8)) */
            int m = 0, st = 0, loc = -99; sscanf(arg, "%d,%d,%d", &m, &st, &loc);   /* (loc: the party there meanwhile, -1 on the map; its reputation shown) */
            u16 tseg = (u16)(LOADSEG + 0x290e), off = rw(tseg, (u16)(0x2937 + 4 * st)), seg = rw(tseg, (u16)(0x2937 + 4 * st + 2));
            u16 a[3] = { (u16)m, 50, 8 }, cl = g_rw(G_CUR_LOC);
            u32 lr = loc >= 0 ? g_location(loc) : 0;
            int set = loc != -99;
            int rep0 = lr ? (s16)rw((u16)(lr >> 16), (u16)((u16)lr + 0x12)) : 0;
            if (set) g_ww(G_CUR_LOC, (u16)loc);
            int ns_call_ptr(u16, u16, int, ...);
            u8 msgs = g_rb(GV_opt_saint_msgs); g_wb(GV_opt_saint_msgs, 0);   /* (no message to wait for its key: no script runs meanwhile) */
            int r = ns_call_ptr(seg, off, 3, a[0], a[1], a[2]);
            g_wb(GV_opt_saint_msgs, msgs);
            if (set) g_ww(G_CUR_LOC, cl);
            fprintf(stderr, "saint %d for member %d: %d", st, m, (s16)r);
            if (lr) fprintf(stderr, " (reputation at %d: %d -> %d)", loc, rep0, (s16)rw((u16)(lr >> 16), (u16)((u16)lr + 0x12)));
            fprintf(stderr, "\n");
        }
        else if (!strcmp(cmd, "ucall")) {               /* test: call sect seg:off (link, hex) with up to 10 int args: ucall:31,18e7,0ab8,-2,80 */
            int sect = 0, a[10] = { 0 }; unsigned sg = 0, of = 0;
            int n = sscanf(arg, "%d,%x,%x,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &sect, &sg, &of, &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6], &a[7], &a[8], &a[9]);
            u16 args[10]; for (int k = 0; k < 10; k++) args[k] = (u16)a[k];
            int v = n >= 3 ? game_call_far(sect, (u16)sg, (u16)of, n - 3 > 0 ? n - 3 : 0, args) : -1;
            fprintf(stderr, "call s%d %04x:%04x: %d\n", sect, sg, of, v);
        }
        else if (!strcmp(cmd, "uarm")) {               /* test (before a battle): member m's missile weapon w (item code), with n of its ammunition (or n more of a thrown weapon) */
            int m = 0, w = 27, n = 20; sscanf(arg, "%d,%d,%d", &m, &w, &n);
            int ammo = w >= 26 && w <= 28 ? 33 : w >= 29 && w <= 30 ? 34 : w >= 31 && w <= 32 ? 35 : w;
            if (m >= 0 && m < 5) {
                g_wb((u16)(G_MEMBER(m) + G_M_MISSILE), (u8)w);
                for (int k = 0; k < 2; k++) {
                    int code = k ? ammo : w, cnt = k ? n : 1, ni = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS));
                    if (k && ammo == w) break;
                    if (ni >= 64) break;
                    u16 o = G_ITEM(m, ni);
                    u32 d = g_itemdef(code); int ty = d ? rb((u16)(d >> 16), (u16)((u16)d + 0x1e)) : 0;   /* (its type, as the game files it: can_fire counts the ammunition by it) */
                    ww(G_FAR2E38, o, (u16)code); wb(G_FAR2E38, (u16)(o + 2), (u8)ty); wb(G_FAR2E38, (u16)(o + 3), 25);   /* (quality 25: a shot takes an arrow of that quality, consume_item) */ wb(G_FAR2E38, (u16)(o + 4), (u8)cnt); wb(G_FAR2E38, (u16)(o + 5), 0);
                    g_wb((u16)(G_MEMBER(m) + G_M_NITEMS), (u8)(ni + 1));
                }
                fprintf(stderr, "arena: member %d armed with %d (ammunition %d x %d)\n", m, w, ammo, n);
            }
        }
        else if (!strcmp(cmd, "uattr")) { int m = 0, a = 0, v = 1; sscanf(arg, "%d,%d,%d", &m, &a, &v); if (m >= 0 && m < 5 && a >= 0 && a < 7) { g_wb((u16)(G_MEMBER(m) + G_M_ATTR_MAX + a), (u8)v); g_wb((u16)(G_MEMBER(m) + G_M_ATTR_CUR + a), (u8)v); } }   /* test: an attribute, now and at most */
        else if (!strcmp(cmd, "udie")) {                /* test: the whole party at no strength, an hour passes (the death screen, the journey's end) */
            for (int m = 0; m < 5; m++) g_wb((u16)(G_MEMBER(m) + G_M_ATTR_CUR + 1), 0);
            u16 h[1] = { 1 }; game_call_far(GF_hours_pass, 1, h);
        }
        else if (!strcmp(cmd, "umoney")) { g_ww(G_MONEY, (u16)atoi(arg)); }   /* test: the party's florins */
        else if (!strcmp(cmd, "uleader")) { int m = atoi(arg); if (m >= 0 && m < 5) g_wb(G_LEADER, (u8)m); fprintf(stderr, "leader now %d\n", (s8)g_rb(G_LEADER)); }   /* test: the party's leader */
        else if (!strcmp(cmd, "utownoffers")) { void citynav_offer_counts(void); citynav_offer_counts(); }   /* test: how many towns have a university, a cathedral ... (the town icons) */
        else if (!strcmp(cmd, "uknows")) {             /* test: what each member knows: formulae (a byte a potion, 3 bits) and saints */
            for (int m = 0; m < 5; m++) {
                fprintf(stderr, "knows %d: formulae", m);
                for (int k = 0; k < 22; k++) fprintf(stderr, " %x", rb(G_FAR2E38, (u16)(0x07be + m * 22 + k)));
                fprintf(stderr, " saints"); for (int k = 0; k < 20; k++) fprintf(stderr, " %02x", rb(G_FAR2E38, (u16)(0x082c + m * 20 + k)));
                fprintf(stderr, "\n");
            }
        }
        else if (!strcmp(cmd, "uitems")) {             /* test: member m's item records: code, type, quality, count (and the missile weapon's ammunition type) */
            int m = atoi(arg);
            if (m >= 0 && m < 5) {
                int n = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS)), w = (s8)g_rb((u16)(G_MEMBER(m) + G_M_MISSILE));
                fprintf(stderr, "items of %d: missile weapon %d (its ammunition type %#x)\n", m, w, w >= 0 && w < 64 ? rb(G_FAR20A5, (u16)(0x7822 + w)) : -1);
                for (int k = 0; k < n && k < 64; k++) { u16 o = G_ITEM(m, k); u32 d = g_itemdef(rw(G_FAR2E38, o) & 0xfff);
                    fprintf(stderr, "  %d: code %d type %#x quality %d count %d (definition +0x1e: %#x)\n", k, rw(G_FAR2E38, o) & 0xfff, rb(G_FAR2E38, (u16)(o + 2)), rb(G_FAR2E38, (u16)(o + 3)), rb(G_FAR2E38, (u16)(o + 4)), d ? rb((u16)(d >> 16), (u16)((u16)d + 0x1e)) : -1); }
            }
        }
        else if (!strcmp(cmd, "uplace")) {             /* test (in battle): combatant i put at x, y (battle map pixels), standing */
            int i = 0, x = 0, y = 0; sscanf(arg, "%d,%d,%d", &i, &x, &y);
            if (i >= 0 && i < 13) { u16 c = (u16)(GV_combatants + i * 0x80);
                g_ww((u16)(c + 4), (u16)x); g_ww((u16)(c + 6), (u16)y); g_ww((u16)(c + 8), (u16)x); g_ww((u16)(c + 0x0a), (u16)y);
                g_wb((u16)(GV_cbt_moving + i), 0); g_ww((u16)(c + 0x10), 0); g_wb((u16)(c + 0x4a), 0xff); g_wb((u16)(GV_cbt_action + i), 7);
                fprintf(stderr, "arena: %d placed at %d,%d\n", i, x, y); }
        }
        else if (!strcmp(cmd, "uarenaplace")) {        /* test (in battle): "i,dx,dy/..." each combatant put at member 0's spot + dx, dy, standing */
            int x0 = (s16)g_rw((u16)(GV_combatants + 4)), y0 = (s16)g_rw((u16)(GV_combatants + 6));
            for (const char *q = arg; *q; ) {
                int i = 0, dx = 0, dy = 0;
                if (sscanf(q, "%d,%d,%d", &i, &dx, &dy) == 3 && i >= 0 && i < 13 && g_rb(CBF(i, CB_STATUS)) == 1) {
                    u16 c = (u16)(GV_combatants + i * 0x80);
                    g_ww((u16)(c + 4), (u16)(x0 + dx)); g_ww((u16)(c + 6), (u16)(y0 + dy)); g_ww((u16)(c + 8), (u16)(x0 + dx)); g_ww((u16)(c + 0x0a), (u16)(y0 + dy));
                    g_wb((u16)(GV_cbt_moving + i), 0);
                    g_ww((u16)(c + 0x10), 0); g_wb((u16)(c + 0x4a), 0xff); g_wb((u16)(GV_cbt_action + i), 7);   /* (no walk left to a spot it stands on: ready, so the AI or the player gives the next) */
                    fprintf(stderr, "arena: %d placed at %d,%d\n", i, x0 + dx, y0 + dy);
                }
                const char *n = strchr(q, '/'); if (!n) break; q = n + 1;
            }
        }
        else if (!strcmp(cmd, "utip")) { void tips_show(int); tips_show(atoi(arg)); }   /* test: a tip window: 1 the start screen's (the New Player Guide), 2 the first battle's, 3 the welcome */
        else if (!strcmp(cmd, "uplaylist")) { void playlist_play(const char *, int); void playlist_seek(int); int n = 0, ms = -1; char dv[16] = "fm"; sscanf(arg, "%d,%d,%15s", &n, &ms, dv);   /* test: the soundtrack's player: piece n (n,-1,DEVICE: on that device, fm by default; n,ms: move to ms in the piece playing) */
            if (ms >= 0) playlist_seek(ms); else playlist_play(dv, n); }
        else if (!strcmp(cmd, "uattack")) { int i = 0, t = 5; sscanf(arg, "%d,%d", &i, &t); void orders_test_attack(int, int); orders_test_attack(i, t); }   /* test (in battle): member i attacks t hand to hand (the player's order) */
        else if (!strcmp(cmd, "uhold")) { int i = 0, on = 1; sscanf(arg, "%d,%d", &i, &on); void orders_test_hold(int, int); orders_test_hold(i, on); }   /* test (in battle): member i told to hold */
        else if (!strcmp(cmd, "uoption")) {               /* test: a switch set as the settings set it (its own effect applied): key=value (uopt is the text click, platform_sdl.c; uset sets the value alone) */
            char k[64]; int v = 0;
            if (sscanf(arg, "%63[^=]=%d", k, &v) == 2) { const struct Option *options_find(const char *); void option_set(const struct Option *, int);
                const struct Option *o = options_find(k); if (o) { option_set(o, v); fprintf(stderr, "option %s = %d\n", k, v); } else fprintf(stderr, "option %s: none\n", k); }
        }
        else if (!strcmp(cmd, "uremove")) {            /* test (in battle): combatant i taken off the field (as fallen, without the fall) */
            int i = atoi(arg); if (i >= 0 && i < 13) { g_wb(CBF(i, CB_STATUS), 0); fprintf(stderr, "arena: %d removed\n", i); }
        }
        else if (!strcmp(cmd, "uarenalog")) { extern int arena_log_every; arena_log_every = atoi(arg); }   /* test: the combatants' state every N ticks (orders.c melee_hook) */
        else if (!strcmp(cmd, "uhit")) {                /* test (in battle): hit_damage(a, t, r, loc), s014 18bf:0778 */
            int a = 0, t = 5, r = 1, loc = 0; sscanf(arg, "%d,%d,%d,%d", &a, &t, &r, &loc);
            u16 args[4] = { (u16)a, (u16)t, (u16)r, (u16)loc };
            int v = game_call_far(14, 0x18bf, 0x0778, 4, args);
            fprintf(stderr, "hit %d on %d: %d (AX %d)\n", a, t, v, (s16)AX);
        }
        else if (!strcmp(cmd, "ukill")) {               /* test (in battle): wounds that fell combatant c, queued (GV_cbt_queued_damage) and taken at once (damage_apply;
                                                           queued alone, the next blow's own roll wrote over them) */
            int c = atoi(arg);
            if (c >= 0 && c < 13) {
                g_wb((u16)(GV_cbt_queued_damage + 2 * c), 100); g_wb((u16)(GV_cbt_queued_damage + 2 * c + 1), 100);
                u16 args[1] = { (u16)c };
                game_call_far(GF_damage_apply, 1, args);
            }
            fprintf(stderr, "kill %d\n", c);
        }
        else if (!strcmp(cmd, "utextdump")) { extern int hdtext_dump; hdtext_dump = 1; }     /* test: as Ctrl+F12, the text verdicts of the next frame */
        else if (!strcmp(cmd, "ulooks")) { int looks_count(void), looks_usable(int); int n = looks_count(), u = 0; for (int k = 0; k < n; k++) u += looks_usable(k); fprintf(stderr, "looks: %d of %d usable\n", u, n); }   /* test: analyse every figure (DL_LOOKS_DUMP) */
        else if (!strcmp(cmd, "uenc")) {                /* test: the next wilderness encounter is new encounter N (mods/encounters.c) */
            void encounters_force(int); int encounters_count(void); const char *encounters_title(int);
            if (!arg[0]) { int n = encounters_count(); for (int k = 0; k < n + 8 && k < 128; k++) if (*encounters_title(k)) fprintf(stderr, "encounter %d: %s\n", k, encounters_title(k)); }
            else encounters_force(atoi(arg));
        }
        else if (!strcmp(cmd, "uwar")) { int l = -1, st = -1; sscanf(arg, "%d,%d", &l, &st); void war_test(int, int); war_test(l, st); }   /* test: the cities at war today; uwar:LOC,STATE forces one (mods/war.c) */
        else if (!strcmp(cmd, "ujumpcard")) { void cutcards_jump(int); cutcards_jump((int)strtol(arg, 0, 0)); }   /* test: the town's next street menu is the game's card N */
        else if (!strcmp(cmd, "ugamecard")) { void cutcards_force(int); cutcards_force((int)strtol(arg, 0, 0)); }   /* test: the next random encounter on the map is the game's card N */
        else if (!strcmp(cmd, "upolit")) {              /* test: a political season starts here (mods/politics.c; the setup s039 1462:0558 and its fix-ups) */
            int politics_test_start(void);
            fprintf(stderr, "politics started at %d: %d\n", (s16)g_rw(G_CUR_LOC), politics_test_start());
        }
        else if (!strcmp(cmd, "upolitage")) { void politics_test_age(int); politics_test_age(atoi(arg)); }   /* test: the season here began N days earlier */
        else if (!strcmp(cmd, "ufind")) { void worldmap_test_find(const char *); worldmap_test_find(arg); }   /* test: the world map's Find, the text then Enter */
        else if (!strcmp(cmd, "uquicksave")) { void saves_quick_save(void); saves_quick_save(); }
        else if (!strcmp(cmd, "uquickload")) { void saves_quick_load(void); saves_quick_load(); }
        else if (!strcmp(cmd, "uload")) { int saves_load(const char *); saves_load(arg); }
        else if (!strcmp(cmd, "ubonus")) {              /* test: the members' bonus bytes (party +0x34) */
            fprintf(stderr, "bonus:");
            for (int m = 0; m < 5; m++) fprintf(stderr, " %02x", g_rb((u16)(G_MEMBER(m) + 0x34)));
            fprintf(stderr, "\n");
        }
        else if (!strcmp(cmd, "uclock")) {              /* test: the clock moves on h hours (no events run) */
            static const int D[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
            int h = (s16)g_rw(G_CUR_HOUR) + atoi(arg), d = (s16)g_rw(G_CUR_DAY), mo = (s16)g_rw(G_CUR_MONTH), y = (s16)g_rw(G_CUR_YEAR);
            while (h >= 24) { h -= 24; if (++d > D[mo]) { d = 1; if (++mo >= 12) { mo = 0; y++; } } }
            g_ww(G_CUR_HOUR, (u16)h); g_ww(G_CUR_DAY, (u16)d); g_ww(G_CUR_MONTH, (u16)mo); g_ww(G_CUR_YEAR, (u16)y);
            fprintf(stderr, "clock %d:00 day %d month %d year %d\n", h, d, mo, y);
        }
        else if (!strcmp(cmd, "uww")) {                /* test: a word of DGROUP (hex offset), set to v when given, then shown */
            unsigned of = 0; int v = 0; int n = sscanf(arg, "%x,%d", &of, &v);
            if (n == 2) g_ww((u16)of, (u16)v);
            fprintf(stderr, "ds:%04x = %d\n", of, (s16)g_rw((u16)of));
        }
        else if (!strcmp(cmd, "uteach")) {             /* test: a teacher of skill k (ability S, fee f) hired here for a week (as the cards make one) */
            int k = 0, S = 30, fee = 50; sscanf(arg, "%d,%d,%d", &k, &S, &fee);
            u16 a[10] = { 0xfffe, g_rw(GV_cur_loc), (u16)S, 0x28, (u16)k, (u16)fee, 0, 168, 0, 0 };
            fprintf(stderr, "teacher of skill %d (ability %d, fee %d) at %d: event %d\n", k, S, fee, (s16)g_rw(GV_cur_loc), game_call_far(GF_event_new, 10, a));
        }
        else if (!strcmp(cmd, "ufar")) {                /* debug: n bytes of far data 20a5 at offset off (hex): ufar:7669,40 */
            unsigned of = 0; int n = 16; sscanf(arg, "%x,%d", &of, &n);
            fprintf(stderr, "far 20a5:%04x:", of);
            for (int k = 0; k < n && k < 256; k++) fprintf(stderr, " %d", rb(G_FAR20A5, (u16)(of + k)));
            fprintf(stderr, "\n");
        }
        else if (!strcmp(cmd, "upoke")) {               /* debug: byte at link-time seg:off (+LOADSEG) */
            unsigned sg = 0, of = 0, v = 0; sscanf(arg, "%x,%x,%x", &sg, &of, &v);
            wb((u16)(LOADSEG + sg), (u16)of, (u8)v);
        }
        else if (!strcmp(cmd, "umode")) { int game_mode(void); fprintf(stderr, "mode %d\n", game_mode()); }   /* 1 card 2 map 3 screen 4 battle */
        else if (!strcmp(cmd, "ugroups")) {             /* test: the enemy groups of the place's levels, the combatants */
            u16 es = g_rw(GV_enemy_seg), go = g_rw(GV_bf_groups);
            for (int l = 0; l < 16; l++) {
                int ng = (s16)g_rw((u16)(GV_group_count + 2 * l)), nm = (s16)g_rw((u16)(GV_mon_count + 2 * l));
                if (!ng && !nm) continue;
                fprintf(stderr, "level %d: %d monsters, %d groups:", l, nm, ng);
                for (int k = 0; k < ng && k < 10; k++) {
                    u16 o = (u16)(go + (l * 10 + k) * 0x1e);
                    if (arg[0] == 'r') { fprintf(stderr, " ["); for (int q = 0; q < 0x1e; q++) fprintf(stderr, "%02x", rb(es, (u16)(o + q))); fprintf(stderr, "]"); continue; }   /* (ugroups:r: the raw records) */
                    fprintf(stderr, " [x6 %d x7 %d t %d/%d/%d n %d/%d/%d x0 %04x x2 %04x]", (s8)rb(es, (u16)(o + 6)), (s8)rb(es, (u16)(o + 7)),
                            (s8)rb(es, (u16)(o + 8)), (s8)rb(es, (u16)(o + 9)), (s8)rb(es, (u16)(o + 10)),
                            (s8)rb(es, (u16)(o + 11)), (s8)rb(es, (u16)(o + 12)), (s8)rb(es, (u16)(o + 13)), rw(es, o), rw(es, (u16)(o + 2)));
                }
                fprintf(stderr, "\n");
            }
            fprintf(stderr, "combatants:");
            for (int i = 5; i < 13; i++) fprintf(stderr, " %d:st%d,fl%d", i, g_rb(CBF(i, CB_STATUS)), (s8)g_rb((u16)(GV_cbt_floor + i)));
            fprintf(stderr, " shown floor %d real %d\n", (s8)g_rb(GV_floor_shown), g_rb(GV_real_combat));
        }
        else if (!strcmp(cmd, "ucg")) {                 /* test: character creation: the professions, and the ones offered now */
            void createview_dump(void); createview_dump();
        }
        else if (!strcmp(cmd, "ucard")) {               /* test: the card's name and its option lines (option id, text) */
            int n = (s8)g_rb(0xe96e);   /* the last card's: it stays set after the card closes ("menu": its options are up) */
            int assist_in_card_menu(void), travel_map_live(void);
            fprintf(stderr, "card %.12s n %d menu %d maplive %d\n", (const char *)M + LIN(G_DGROUP, 0xe3d4), n, assist_in_card_menu(), travel_map_live());
            for (int i = 0; i < n && i < 20; i++) {
                int y = g_rb((u16)(0xe974 + i)), h = g_rb((u16)(0xe988 + i)), opt = (s8)g_rb((u16)(0xe9b0 + i));
                char t[256] = "";
                for (int k = 0; k < hdtext_nruns; k++)
                    if (hdtext_runs[k].y >= y - 1 && hdtext_runs[k].y < y + h - 1 && hdtext_runs[k].x >= 60 && strlen(t) + strlen(hdtext_runs[k].text) < 250) { if (*t) strcat(t, " | "); strcat(t, hdtext_runs[k].text); }
                fprintf(stderr, "  line %d opt %d y %d h %d on %d: %s\n", i, opt, y, h, opt >= 0 && opt < 20 ? g_rb((u16)(0xe99c + opt)) : -1, t);
            }
        }
        else if (!strcmp(cmd, "ualchemy")) { extern int uscreen_pending, uscreen_a, uscreen_b; uscreen_a = 0; uscreen_b = 8; sscanf(arg, "%d,%d", &uscreen_a, &uscreen_b); uscreen_pending = 1; }   /* test: the alchemy screen (member, hours) at the next card menu */
        else if (!strcmp(cmd, "uunpause")) { int p = g_rb(0xa695) == 1; if (p) { void bios_kbd_inject(u16); bios_kbd_inject(0x3920); } fprintf(stderr, "unpause: %s\n", p ? "was paused" : "running"); }   /* test: a battle running (Space only if it is paused: ai_paused) */
        else if (!strcmp(cmd, "umenu")) { int m = 0, i = 0, j = -1; sscanf(arg, "%d,%d,%d", &m, &i, &j); void menuview_request(int, int, int); menuview_request(m, i, j); }   /* test: menu m's entry i (sub-entry j) chosen, as a click on it does */
        else if (!strcmp(cmd, "ulib")) {                /* test: the saints the library at location k teaches */
            int k = atoi(arg), o[4]; int gm_library_saints(int loc, int out[4]); gm_library_saints(k, o);
            fprintf(stderr, "library %d: %d %d %d %d\n", k, o[0], o[1], o[2], o[3]);
        }
        else if (!strcmp(cmd, "uevset")) {             /* test: word +off of event i set to v (hex off) */
            int i = 0, v = 0; unsigned off = 0; sscanf(arg, "%d,%x,%d", &i, &off, &v);
            u32 e = i >= 0 ? ev_far(i) : 0;
            if (e) ww((u16)(e >> 16), (u16)((u16)e + off), (u16)v);
            fprintf(stderr, "uevset %d +%x = %d%s\n", i, off, v, e ? "" : " (no event)");
        }
        else if (!strcmp(cmd, "uevents")) {             /* test: the events of a kind (-1 all): index, kind, type0, owner +1a, loc +1c, value +1e, tag +28, +2a, +2c */
            int k = -1; sscanf(arg, "%d", &k);
            for (int i = 0; i < 300; i++) { u32 e = ev_far(i); if (!e) continue; u16 sg = (u16)(e >> 16), o = (u16)e;
                if (k != -1 && (s16)rw(sg, (u16)(o + 0x22)) != k) continue;
                fprintf(stderr, "event %d: kind %#x type0 %d owner %d loc %d value %d tag %d aux %d %d\n", i, (s16)rw(sg, (u16)(o + 0x22)), (s16)rw(sg, o), (s16)rw(sg, (u16)(o + 0x1a)),
                        (s16)rw(sg, (u16)(o + 0x1c)), (s16)rw(sg, (u16)(o + 0x1e)), (s16)rw(sg, (u16)(o + 0x28)), (s16)rw(sg, (u16)(o + 0x2a)), (s16)rw(sg, (u16)(o + 0x2c))); }
        }
        else if (!strcmp(cmd, "uoriginal")) { void ui_set_original_c(int); ui_set_original_c(atoi(arg)); }   /* test: the original game (the Full Original preset's "original") on (1) or off (0) */
        else if (!strcmp(cmd, "ualcrisk")) { int r = 0, n = 200; sscanf(arg, "%d,%d", &r, &n); alc_risk_test(r, n); }   /* test: n failed brews of risk r here: the accidents */
        else if (!strcmp(cmd, "ualc")) {                /* test: the formulae town k's alchemist sells */
            int k = atoi(arg), o[4], p[4]; extern int ui_alchemist_formulae_c(int, int *, int *);
            if (ui_alchemist_formulae_c(k, o, p)) fprintf(stderr, "alchemist %d: %d %d %d %d (%d %d %d %d fl)\n", k, o[0], o[1], o[2], o[3], p[0], p[1], p[2], p[3]);
            else fprintf(stderr, "alchemist %d: none\n", k);
        }
        else if (!strcmp(cmd, "ulocw")) {               /* test: word at +off of location k's record (ulocw:k,off,v; v -999 only reads) */
            int k = 0, of = 0, v = -999; sscanf(arg, "%d,%x,%d", &k, &of, &v);
            u32 r = g_location(k);
            if (r) { fprintf(stderr, "loc %d +%x: %d", k, of, (s16)rw((u16)(r >> 16), (u16)((u16)r + of)));
                     if (v != -999) { ww((u16)(r >> 16), (u16)((u16)r + of), (u16)v); fprintf(stderr, " -> %d", v); } fprintf(stderr, "\n"); }
        }
        else if (!strcmp(cmd, "ucache")) { extern int uscreen_pending; uscreen_pending = 3; }   /* test: the store room from the card menu up */
        else if (!strcmp(cmd, "usaint")) { extern int uscreen_pending, uscreen_a, uscreen_b; uscreen_a = 0; uscreen_b = 0; sscanf(arg, "%d,%d", &uscreen_a, &uscreen_b); uscreen_pending = 2; }   /* test: the prayer screen (member, saint) from the card menu up */
        else if (!strcmp(cmd, "unspush")) { int t = 0, x = 0, y = 0, z = 0; sscanf(arg, "%d,%d,%d,%d", &t, &x, &y, &z); void ns_push(int, int, int, int); ns_push(t, x, y, z); }   /* test: an action for the native screen up (mods/barterview.h BV_*, sheetview.h SV_*) */
        else if (!strcmp(cmd, "uitem")) {               /* test: member m gets n of item id at quality q (a new record; ready it with ucall:31,18e7,11b2,m,record) */
            int m = 0, id = 0, q = 25, n = 1; sscanf(arg, "%d,%d,%d,%d", &m, &id, &q, &n);
            int k = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS)); u32 d = g_itemdef(id);
            int same = -1;                                  /* (a stack of the same: added to it, as add_item_rec does) */
            for (int j = 0; j < k && j < 64; j++) if (rw(G_FAR2E38, G_ITEM(m, j)) == (u16)id && rb(G_FAR2E38, (u16)(G_ITEM(m, j) + 3)) == (u8)q) same = j;
            if (d && same >= 0) { u16 o = G_ITEM(m, same); wb(G_FAR2E38, (u16)(o + 4), (u8)(rb(G_FAR2E38, (u16)(o + 4)) + n)); fprintf(stderr, "item %d x%d to member %d (onto record %d)\n", id, n, m, same); }
            else if (d && k < 53) { u16 o = G_ITEM(m, k); ww(G_FAR2E38, o, (u16)id); wb(G_FAR2E38, (u16)(o + 2), rb((u16)(d >> 16), (u16)((u16)d + 0x1e))); wb(G_FAR2E38, (u16)(o + 3), (u8)q); wb(G_FAR2E38, (u16)(o + 4), (u8)n); wb(G_FAR2E38, (u16)(o + 5), rb((u16)(d >> 16), (u16)((u16)d + 0x25))); g_wb((u16)(G_MEMBER(m) + G_M_NITEMS), (u8)(k + 1)); fprintf(stderr, "item %d x%d to member %d (record %d)\n", id, n, m, k); }
        }
        else if (!strcmp(cmd, "uinv")) {                /* test: member m's item records (code, type, quality, count) */
            int m = atoi(arg), n = (s8)g_rb((u16)(G_MEMBER(m) + G_M_NITEMS));
            fprintf(stderr, "inv %d (%d):", m, n);
            for (int k = 0; k < n && k < 64; k++) { u16 o = G_ITEM(m, k); fprintf(stderr, " [%d] %d/%d q%d x%d", k, rw(G_FAR2E38, o), rb(G_FAR2E38, (u16)(o + 2)), rb(G_FAR2E38, (u16)(o + 3)), rb(G_FAR2E38, (u16)(o + 4))); }
            u16 cb = (u16)(0x9c55 + m * 0x80);
            fprintf(stderr, " | armor %d q%d, %d q%d\n", (s8)g_rb((u16)(cb + 0x4b)), (s8)g_rb((u16)(cb + 0x4f)), (s8)g_rb((u16)(cb + 0x4c)), (s8)g_rb((u16)(cb + 0x50)));
        }
        else if (!strcmp(cmd, "urand")) { unsigned v = (unsigned)atol(arg); g_ww(0x7b20, (u16)v); g_ww(0x7b22, (u16)(v >> 16)); fprintf(stderr, "rand seed %u\n", v); }   /* test: the C library's rand() state */
        else if (!strcmp(cmd, "ucv")) { void cardview_dump(void); cardview_dump(); }      /* test: the native card screen */
        else if (!strcmp(cmd, "uhot")) { int n = (s8)g_rb(0xe0a1); fprintf(stderr, "hots %d, order mode %d member %d action %d\n", n, g_rb(0xa6b3), (s8)g_rb(0xa6b2), (s16)g_rw(0xa6b0)); for (int k = 0; k < n && k < 40; k++) fprintf(stderr, "hot %d cbt %d x %d..%d y %d..%d\n", k, g_rb((u16)(0xd848 + k)), (s16)g_rw((u16)(0xd8f1 + 2 * k)), (s16)g_rw((u16)(0xd8d7 + 2 * k)), (s16)g_rw((u16)(0xd8bd + 2 * k)), (s16)g_rw((u16)(0xd8a3 + 2 * k))); }
        else if (!strcmp(cmd, "utgt")) { int mx, my; plat_mouse_game(&mx, &my); u8 t = g_rb(0xd845); if (t != 255) fprintf(stderr, "tgt %d at %d,%d\n", (s8)t, mx, my); }
        else if (!strcmp(cmd, "ucbt")) {               /* test: the battle's combatants and what the pointer is on */
            fprintf(stderr, "cbt target %d\n", (s16)g_rw(0xd845));
            for (int i = 0; i < 13; i++) { u16 b = (u16)(0x9c55 + i * 0x80); char n[26]; g_str(G_DGROUP, (u16)(b + 0x25), n, sizeof n);
                fprintf(stderr, "  %d st %d %s end %d/%d str %d/%d w %d c23 %d c51 %d c4b %d rec3d %d rec0e %d rec37 %d atk %d def %d xy %d,%d order %#x target %d act %d moving %d to %d,%d q %d (%d/%d) arm %02x\n", i, g_rb((u16)(b + 0x14)), n, (s8)g_rb((u16)(b + 0x5d)), (s8)g_rb((u16)(b + 0x64)),
                        (s8)g_rb((u16)(b + 0x5e)), (s8)g_rb((u16)(b + 0x65)), (s8)g_rb((u16)(b + 0x22)), (s8)g_rb((u16)(b + 0x23)), (s8)g_rb((u16)(b + 0x51)), (s8)g_rb((u16)(b + 0x4b)), (s8)g_rb((u16)(G_MEMBER(i) + G_M_MELEE)), (s8)g_rb((u16)(G_MEMBER(i) + G_M_MISSILE)), (s8)g_rb((u16)(G_MEMBER(i) + G_M_ARMOR_BODY)), g_rb((u16)(b + 0x21)), g_rb((u16)(b + 0x20)), (s16)g_rw((u16)(b + 4)), (s16)g_rw((u16)(b + 6)),
                        g_rw((u16)(b + 0x10)), (s8)g_rb((u16)(b + 0x4a)), g_rb((u16)(GV_cbt_action + i)), g_rb((u16)(GV_cbt_moving + i)), (s16)g_rw((u16)(b + 8)), (s16)g_rw((u16)(b + 10)),
                        (s8)g_rb((u16)(b + 0x0d)), (s8)g_rb((u16)(b + 0x58)), (s8)g_rb((u16)(b + 0x5a)), g_rb((u16)(b + 0x55))); }
        }
        else if (!strcmp(cmd, "uparty")) {             /* test: End/Str of each member, battle pause flag */
            fprintf(stderr, "party %u ms paused %d mode %d:", plat_ticks_ms(), g_rb(0x07d3), game_mode());
            for (int m = 0; m < 5; m++) fprintf(stderr, " %d/%d,%d/%d", (s8)g_rb((u16)(G_MEMBER(m) + G_M_ATTR_CUR)), (s8)g_rb((u16)(G_MEMBER(m) + G_M_ATTR_MAX)), (s8)g_rb((u16)(G_MEMBER(m) + G_M_ATTR_CUR + 1)), (s8)g_rb((u16)(G_MEMBER(m) + G_M_ATTR_MAX + 1)));
            fprintf(stderr, "\n");
        }
        else if (!strcmp(cmd, "uattrs")) {             /* test: member m's attributes (current/max) and skills */
            int m = atoi(arg); if (m < 0 || m > 4) m = 0;
            fprintf(stderr, "attrs %d:", m);
            for (int a = 0; a < 7; a++) fprintf(stderr, " %d/%d", (s8)g_rb((u16)(G_MEMBER(m) + G_M_ATTR_CUR + a)), (s8)g_rb((u16)(G_MEMBER(m) + G_M_ATTR_MAX + a)));
            fprintf(stderr, " skills:");
            for (int k = 0; k < 19; k++) fprintf(stderr, " %d", (s8)g_rb((u16)(G_MEMBER(m) + G_M_SKILLS + k)));
            fprintf(stderr, "\n");
        }
        else if (!strcmp(cmd, "uexport")) { char w[1300]; int datamods_export_cards(char *, size_t); int r = datamods_export_cards(w, sizeof w); fprintf(stderr, "export: %d to %s\n", r, w); }
        else if (!strcmp(cmd, "umissile")) { int i = 0, t = 5; sscanf(arg, "%d,%d", &i, &t); void battlerules_test_missile(int, int); battlerules_test_missile(i, t); }   /* test: member i shoots at t (the missile order) */
        else if (!strcmp(cmd, "ucardhas")) {            /* test: ucardhas:$VILLA02,13,$SabbatLocation (a card block's raw text) */
            char card[16] = "", word[40] = ""; int b = 0; sscanf(arg, "%15[^,],%d,%39s", card, &b, word);
            int citynav_card_block_has(const char *, int, const char *); fprintf(stderr, "cardhas %s %d %s: %d\n", card, b, word, citynav_card_block_has(card, b, word));
        }
        else if (!strcmp(cmd, "uquests")) { void quests_test_dump(void); quests_test_dump(); }   /* test: the tasks, printed */
        else if (!strcmp(cmd, "udlg")) { extern int cardview_dialog, game_dialog_kind; extern char game_dialog_text[600]; snprintf(game_dialog_text, sizeof game_dialog_text, "%s", arg); for (char *q = game_dialog_text; *q; q++) if (*q == '_') *q = ' '; game_dialog_kind = *arg ? 2 : 0; cardview_dialog = *arg ? 1 : 0; }   /* test: the native box for a game message (udlg:Title_-_text; udlg: off) */
        else if (!strcmp(cmd, "ushops")) { void journal_test_shops(int); journal_test_shops(atoi(arg)); }   /* test: the shops noted in town N (-1 here) and their wares */
        else if (!strcmp(cmd, "usnddrv")) { void soundpreview_test_load(int); soundpreview_test_load(atoi(arg)); }   /* test: the game's sound driver loaded again as the preview does (0 the towns', 1 the battles') */
        else if (!strcmp(cmd, "umark")) { u32 plat_ticks_ms(void); fprintf(stderr, "mark %s %u\n", arg, plat_ticks_ms()); }   /* test: a line in the log at this moment (with the time), to find a test's steps in it */
        else if (!strcmp(cmd, "uwhere")) { void journal_test_where(int); journal_test_where(atoi(arg)); }   /* test: where item N is sold, days to some towns */
        else if (!strcmp(cmd, "usabbat")) { void quests_test_learn(int); quests_test_learn(atoi(arg)); }   /* test: as if a card told of the next Sabbat (1 place, 2 night) */
        else if (!strcmp(cmd, "ucityplaces")) { void citynav_check_places(void); citynav_check_places(); }   /* test: every place card's option lists through Go to's way out */
        else if (!strcmp(cmd, "ugotoplaces")) { void citynav_test_places(void); citynav_test_places(); }   /* test: the places of this town (Go to's list) */
        else if (!strcmp(cmd, "ulivecard")) { void citynav_test_live(void); citynav_test_live(); }   /* test: the card's text as loaded against MSGFILES */
        else if (!strcmp(cmd, "ucitylines")) { void citynav_check_lines(void); citynav_check_lines(); }   /* test: every hub card line through the Go to rules; unmatched ones logged */
        else if (!strcmp(cmd, "ugoto")) { void citynav_go(int); citynav_go(atoi(arg)); }   /* test: Go to place N in the city */
        else if (!strcmp(cmd, "uhelp")) { void ui_toggle_help(void); ui_toggle_help(); }
        else if (!strcmp(cmd, "ugotowin")) { void ui_open_goto(void); ui_open_goto(); }       /* test: the Go to window (G) */
        else if (!strcmp(cmd, "umap")) { void ui_toggle_map(void); ui_toggle_map(); }
        else if (!strcmp(cmd, "utravel")) {             /* test: auto-travel to place loc (the world map's Travel there); the time as it sets off */
            int travel_to_loc(int, const char *), travel_minutes_loc(int, int *, int *); int l = atoi(arg);
            fprintf(stderr, "utravel %d: %d minutes planned, %d (day %d hour %d)\n", l, travel_minutes_loc(l, 0, 0), travel_to_loc(l, "test"), (s16)g_rw(G_CUR_DAY), (s16)g_rw(G_CUR_HOUR)); }
        else if (!strcmp(cmd, "ujobs")) {               /* test: the day jobs' table (DS:3ace, 31 x 18 bytes; names F_290e_219b) */
            for (int i = 0; i < 31; i++) {
                u16 o = (u16)(0x3ace + i * 18);
                int ni = g_rb(o);
                u16 no = rw((u16)(LOADSEG + 0x290e), (u16)(0x219b + ni * 4)), ns = rw((u16)(LOADSEG + 0x290e), (u16)(0x219b + ni * 4 + 2));
                char nm[40]; g_str(ns, no, nm, sizeof nm);
                fprintf(stderr, "job %2d %-22s cats %08x/%08x flags %02x attr %d>=%d sk %d>=%d sk %d>=%d pay %d\n", i, nm,
                        (unsigned)(g_rw((u16)(o + 2)) | (u32)g_rw((u16)(o + 4)) << 16), (unsigned)(g_rw((u16)(o + 6)) | (u32)g_rw((u16)(o + 8)) << 16),
                        g_rb((u16)(o + 10)), g_rb((u16)(o + 11)), g_rb((u16)(o + 12)), g_rb((u16)(o + 13)), g_rb((u16)(o + 14)), g_rb((u16)(o + 15)), g_rb((u16)(o + 16)), g_rb((u16)(o + 17)));
            } }
        else if (!strcmp(cmd, "uwhen")) fprintf(stderr, "when: day %d month %d hour %d, place %d\n", (s16)g_rw(G_CUR_DAY), (s16)g_rw(G_CUR_MONTH), (s16)g_rw(G_CUR_HOUR), (s16)g_rw(G_CUR_LOC));   /* test: the date */
        else if (!strcmp(cmd, "urestart")) { void plat_restart(void); if (!getenv("DL_RESTARTED")) plat_restart(); }   /* test: start again (once) */
    else return 0;
    return 1;
}

/* the arena's log (uarenalog:N): every N ticks of the battle clock, each combatant standing - where,
   its order and target, action, state, walking, its weapon in hand, Endurance, and the foes beside it -
   one line for tools/arena.py to read */
int arena_log_every;
void arena_tick(void)
{
    static u16 last;
    u16 now = g_rw(GV_combat_clock);
    if (arena_log_every <= 0 || (u16)(now - last) < (u16)arena_log_every) return;
    last = now;
    fprintf(stderr, "ARENA t=%u", now);
    for (int i = 0; i < 13; i++) {
        u16 c = (u16)(GV_combatants + i * 0x80);
        if (g_rb((u16)(c + 0x14)) != 1) continue;
        int beside = 0; for (int j = 0; j < 13; j++) if (j != i && (i < 5) != (j < 5) && g_rb(CBF(j, CB_STATUS)) == 1 && rb(G_FAR20A5, (u16)(0x7900 + i * 13 + j))) beside++;
        fprintf(stderr, " |%d:%d,%d o%x t%d a%d s%d m%d w%d e%d b%d", i, (s16)g_rw((u16)(c + 4)), (s16)g_rw((u16)(c + 6)), g_rw((u16)(c + 0x10)), (s8)g_rb((u16)(c + 0x4a)),
                g_rb((u16)(GV_cbt_action + i)), i < 5 ? g_rb((u16)(GV_cbt_state + i)) : 0, g_rb((u16)(GV_cbt_moving + i)), (s8)g_rb((u16)(c + 0x22)), (s8)g_rb((u16)(c + 0x5d)), beside);
    }
    fprintf(stderr, "\n");
}
