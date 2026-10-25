/* playlist.c - the soundtrack, piece by piece, in F12 > Settings > Sound (ui/settings_setup.cpp):
 * each piece of the game's music (8 ... 47, named as mods/musicpack.c names them) played by the game's
 * own music driver through the device chosen (mods/soundpreview.c changes the driver for it), with
 * play and pause, the time played, its length, and a bar to move through it.
 *
 * Time: the timer interrupts delivered while it plays (runtime/bios.c music_ticks: the driver counts
 * its tempo in them). Pause: none delivered (music_hold), every key let go. Moving forward: a burst
 * of them at once with the notes held back (music_ff: runtime/mpu.c drops the note-ons, runtime/sound.c
 * keeps the FM keys off); back: the piece started again and run forward.
 * Length: the first time a piece is played on a device it is run ahead silently, over a few frames,
 * and its notes noted (playlist_heard): a piece that loops is heard to repeat its notes from some
 * point, one that ends falls silent. Kept in the settings (music.len.DEVICE.N, milliseconds; -2:
 * repeats, no length found). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/config.h"
#include "nscreen.h"
#include "musicpack.h"
#include "playlist.h"

extern volatile u32 music_ticks; extern int music_hold, music_ff;
void bios_timer_burst(int n); double bios_tick_us(void);
void soundpreview_piece(const char *dev, int n); void soundpreview_replay(int n); void soundpreview_hush(void);
void soundpreview_stop(void); int soundpreview_active(void);
u32 plat_ticks_ms(void);

static char dev_now[16];
static int now_n = -1, paused, measuring;
static u32 base;                                 /* music_ticks when the piece began (its time: music_ticks - base) */

/* ---------------------------------------------------------------- the notes heard, for a length */
#define MAXEV 8000
static struct { u32 t; u32 key; } ev[MAXEV];
static int nev;
static u32 mbase, measured_ticks;
void playlist_heard(int key) { if (measuring && nev < MAXEV) { ev[nev].t = music_ticks - mbase; ev[nev].key = (u32)key; nev++; } }

static u32 ticks_of_ms(double ms) { double us = bios_tick_us(); return us > 0 ? (u32)(ms * 1000.0 / us) : 0; }
static int ms_of_ticks(u32 t) { return (int)(t * bios_tick_us() / 1000.0); }

/* where the notes begin again as they began (the same notes after the same gaps, for a good stretch):
   the piece's length up to there, in ticks; 0 not yet heard */
static int by_time(const void *a, const void *b)
{
    const u32 *x = (const u32 *)a, *y = (const u32 *)b;      /* (t, key): notes struck together in one order, whatever order the driver struck them */
    return x[0] != y[0] ? (x[0] < y[0] ? -1 : 1) : (int)x[1] - (int)y[1];
}
static u32 repeat_at(void)
{
    qsort(ev, (size_t)nev, sizeof ev[0], by_time);
    const int K = 48; u32 min_gap = ticks_of_ms(6000);
    for (int i0 = 0; i0 < 12 && i0 < nev; i0++)             /* (from one of the first notes: a piece may have an opening not played again) */
        for (int j = i0 + 1; j + K <= nev; j++) {
            if (ev[j].key != ev[i0].key || ev[j].t - ev[i0].t < min_gap) continue;
            int m = 1;
            for (; m < K; m++) {
                if (ev[j + m].key != ev[i0 + m].key) break;
                long d = (long)(ev[j + m].t - ev[j].t) - (long)(ev[i0 + m].t - ev[i0].t);
                if (d < -2 || d > 2) break;
            }
            if (m == K) return ev[j].t - ev[i0].t + ev[0].t;
        }
    return 0;
}

/* the pieces' lengths found by running each ahead as above on the game's own music (the same in
   every copy), on each driver (the MT-32's serves an outside synthesizer too): shown before a piece
   is first played, and no silent run ahead for it. A length measured on the machine (music.len.*)
   is taken first. ms; -2 repeats (no length found); 0 none known */
static const int LEN_MT32[40] = {   /* pieces 8 ... 47 */
    58889, 106709, 58666, 66806, 42269, 28648, 60620, 61857, 62120, 70058,
    44594, 120266, 80002, -2, 94045, 70105, 8660, 11546, -2, 2582,
    1500, 58099, 2445, 11546, 11546, 1946, 1500, -2, 17892, 30263,
    -2, -2, -2, 81364, 14696, 51295, 1000, 55797, 82888, -2 };
static const int LEN_FM[40] = {   /* pieces 8 ... 47 */
    31741, 106709, 59542, -2, -2, 31741, 60620, 60207, 62120, 98925,
    41002, 120266, 80002, -2, 94045, 70105, 8660, 11546, 184747, 2702,
    52635, 58718, 2874, 40413, -2, 2187, 1500, -2, 17892, 30263,
    -2, -2, -2, 81364, 17170, 51295, 1000, 69680, 82476, -2 };
static const int LEN_ADLIB[40] = {   /* pieces 8 ... 47 */
    68432, 124203, 68172, 77914, -2, 33162, 70700, 72142, 72240, 82300,
    47571, 140014, 93304, -2, 109433, 81761, 6733, 13466, -2, 2902,
    61138, 68232, 3103, 6733, 13747, 2000, 1500, 40400, 20978, 36008,
    -2, -2, 282800, 94804, 18814, 59575, 1000, 81017, 96671, 0 };
static int known_len(const char *dev, int n)
{
    if (n < 8 || n > 47) return -1;
    const int *t = !strcmp(dev, "mt32") || !strcmp(dev, "midi") ? LEN_MT32 : !strcmp(dev, "fm") ? LEN_FM : !strcmp(dev, "adlib") ? LEN_ADLIB : 0;
    return t && t[n - 8] ? t[n - 8] : -1;
}
static void key_of(int n, char *k, int sz) { snprintf(k, (size_t)sz, "music.len.%s.%d", dev_now, n); }
int playlist_length_dev_ms(const char *dev, int n)
{
    if (!dev || !dev[0]) return -1;
    char k[48]; snprintf(k, sizeof k, "music.len.%s.%d", dev, n);
    int v = config_get_int(k, -1);
    return v != -1 ? v : known_len(dev, n);
}
int playlist_length_ms(int n) { return playlist_length_dev_ms(dev_now, n); }   /* -1 not known on this device, -2 repeats (no length found) */

static void measure_step(int n, int b, int c, int d)
{
    (void)b; (void)c; (void)d;
    if (!measuring || now_n != n || !soundpreview_active()) { measuring = 0; music_ff = 0; return; }
    u32 t0 = plat_ticks_ms();
    music_ff = 1;
    u32 mt0 = music_ticks;
    while (plat_ticks_ms() - t0 < 10) { bios_timer_burst(64); measured_ticks += 64; }   /* (some ten milliseconds a frame: the window keeps drawing) */
    music_ff = 0;
    if (music_ticks == mt0) { ns_defer(measure_step, n, 0, 0, 0); measured_ticks = music_ticks - mbase; return; }   /* (no interrupt could be delivered now: the next frame) */
    if (getenv("DL_PLAYLIST_LOG")) { int opl_write_count(void); fprintf(stderr, "playlist: measuring %d: %u ticks, %d notes, %d FM writes so far\n", n, measured_ticks, nev, opl_write_count()); }
    int len = -1; u32 cur = measured_ticks;
    if (musicpack_loops(n)) {
        u32 r = repeat_at();
        if (r) len = ms_of_ticks(r);
        else if (cur > ticks_of_ms(8 * 60000.0)) len = -2;
    } else {
        u32 last = nev ? ev[nev - 1].t : 0;
        if (nev && cur - last > ticks_of_ms(12000)) len = ms_of_ticks(last) + 1500;
        else if (!nev && cur > ticks_of_ms(30000)) len = 1000;
    }
    if (len == -1) { ns_defer(measure_step, n, 0, 0, 0); return; }
    char k[48]; key_of(n, k, sizeof k); config_set_int(k, len); config_save();
    fprintf(stderr, "playlist: piece %d on %s: %d ms%s (%d notes heard in %d s)\n", n, dev_now, len, len == -2 ? " (repeats, no length found)" : "", nev, ms_of_ticks(cur) / 1000);
    measuring = 0;
    soundpreview_replay(n);                      /* from its beginning, now heard */
    base = music_ticks; music_hold = paused = 0;
}

void playlist_started(int n)                     /* (mods/soundpreview.c: the driver for the device loaded, piece n begun) */
{
    now_n = n; paused = 0; music_hold = 0; base = music_ticks;
    if (playlist_length_ms(n) == -1) {           /* not measured on this device: run it ahead first, silently */
        measuring = 1; nev = 0; mbase = music_ticks; measured_ticks = 0; music_hold = 1;
        ns_defer(measure_step, n, 0, 0, 0);
    }
}
static void replay_now(int n, int b, int c, int d) { (void)b; (void)c; (void)d; soundpreview_replay(n); playlist_started(n); }

void playlist_play(const char *dev, int n)
{
    if (strcmp(dev, dev_now) || !soundpreview_active()) {
        snprintf(dev_now, sizeof dev_now, "%s", dev);
        now_n = n; measuring = 0; music_ff = 0; music_hold = 0;
        soundpreview_piece(dev, n);
    } else { now_n = n; measuring = 0; ns_defer(replay_now, n, 0, 0, 0); }
}
static void hush_now(int a, int b, int c, int d) { (void)a; (void)b; (void)c; (void)d; soundpreview_hush(); }
void playlist_pause(int on)
{
    if (now_n < 0 || measuring) return;
    paused = on; music_hold = on;
    if (on) ns_defer(hush_now, 0, 0, 0, 0);
}
void playlist_stop(void)
{
    if (now_n < 0) return;
    now_n = -1; measuring = 0; music_ff = 0; music_hold = 0; paused = 0;
    soundpreview_stop();
}

/* the time played in the piece, ms (a piece that repeats: within its length) */
int playlist_pos_ms(void)
{
    if (now_n < 0 || measuring) return 0;
    int t = ms_of_ticks(music_ticks - base), len = playlist_length_ms(now_n);
    if (len > 0 && musicpack_loops(now_n)) t %= len;
    else if (len > 0 && t > len) t = len;
    return t;
}
static void seek_now(int ms, int b, int c, int d)
{
    (void)b; (void)c; (void)d;
    if (now_n < 0 || measuring || !soundpreview_active()) return;
    u32 target = ticks_of_ms(ms), pos = music_ticks - base;
    int len = playlist_length_ms(now_n);
    u32 in = len > 0 && musicpack_loops(now_n) ? pos % ticks_of_ms(len) : pos;
    if (target < in) { soundpreview_replay(now_n); in = 0; }   /* (back: from the beginning, then forward) */
    u32 go = target - in;
    music_ff = 1;
    while (go > 0) { int k = go > 256 ? 256 : (int)go; bios_timer_burst(k); go -= (u32)k; }
    music_ff = 0;
    soundpreview_hush();
    base = music_ticks - target;
}
void playlist_seek(int ms) { if (now_n >= 0 && !measuring) ns_defer(seek_now, ms, 0, 0, 0); }

int playlist_now(void) { return soundpreview_active() ? now_n : -1; }
int playlist_paused(void) { return paused; }
int playlist_measuring(void) { return measuring; }

/* each frame (ui/ui.cpp): the player stops when the settings close, or when its preview ended */
void playlist_frame(int settings_shown)
{
    if (now_n < 0) return;
    if (!soundpreview_active()) { now_n = -1; measuring = 0; music_ff = 0; music_hold = 0; paused = 0; return; }
    if (!settings_shown) playlist_stop();
    if (getenv("DL_PLAYLIST_LOG")) { static u32 t; u32 n2 = plat_ticks_ms(); if (n2 - t > 2000) { int opl_write_count(void); t = n2; fprintf(stderr, "playlist: playing %d at %d ms (%u ticks, %d FM writes, hold %d)\n", now_n, playlist_pos_ms(), music_ticks, opl_write_count(), music_hold); } }
}
