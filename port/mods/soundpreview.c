/* soundpreview.c - hearing a sound device before choosing it (F12 > Settings):
 * a piece of music, or ten seconds of a real battle's sounds, played through that device now, then the chosen one
 * back. The game's own way of changing its sound driver (as it does going into a battle and out,
 * s002 0e76): stop it (047c:0791, 00a9), let go of it (08f9), load xSOUND.DLC or .DLB (07ce: the
 * driver file is picked by the device, runtime/sounddev.c, which reads the device to hear from
 * here meanwhile), take it in (08c8), set it up (0783 with the card's port, IRQ, DMA) and start it
 * (0066); then sound_play(n). Not during a battle (its driver is busy); the music pack is held
 * back meanwhile, so the device itself is heard. Also the packs' own previews (a recording from
 * the music pack, the battle sounds pack). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "nscreen.h"

u32 plat_ticks_ms(void);
static char over[16];                    /* the device heard during a preview ("" none) */
const char *sound_device_override(void) { return over[0] ? over : 0; }
int musicpack_held;                      /* (mods/musicpack.c: the pack quiet while a device is heard) */

static int call(u16 off, int n, u16 a0, u16 a1, u16 a2, u16 a3, u16 a4, u16 a5, u16 a6)
{
    u16 a[7] = { a0, a1, a2, a3, a4, a5, a6 };
    return (s16)game_call_far(-1, 0x047c, off, n, a);
}
/* the game's sound driver changed for the one of the device now in force: the towns' (0) or the battles' (1) */
static void load_driver(int battle)
{
    char letter = (char)g_rb(0xa876);
    if (letter == 'n' || letter == 'N' || !letter) letter = 'r';      /* (the game set to no sound: the device chosen here decides anyway) */
    u8 keep[16]; for (int i = 0; i < 16; i++) keep[i] = g_rb((u16)(GV_msg_buf + i));
    char nm[16]; snprintf(nm, sizeof nm, "%csound.%s", letter, battle ? "dlb" : "dlc");
    call(0x0791, 0, 0, 0, 0, 0, 0, 0, 0);
    call(0x00a9, 0, 0, 0, 0, 0, 0, 0, 0);
    call(0x08f9, 1, g_rw(0x9417), 0, 0, 0, 0, 0, 0);
    for (int i = 0; i < 16; i++) g_wb((u16)(GV_msg_buf + i), (u8)nm[i]);
    int h = call(0x07ce, 2, GV_msg_buf, 0, 0, 0, 0, 0, 0);
    g_ww(0x9417, (u16)h);
    call(0x08c8, 1, (u16)h, 0, 0, 0, 0, 0, 0);
    call(0x0783, 7, 0, 0, 0, g_rw(0xa870), g_rw(0xa86e), g_rw(0xa872), g_rw(0xa874));
    call(0x0066, 0, 0, 0, 0, 0, 0, 0, 0);
    for (int i = 0; i < 16; i++) g_wb((u16)(GV_msg_buf + i), keep[i]);
}
static void play(int n) { call(0x078a, 1, (u16)n, 0, 0, 0, 0, 0, 0); }
void opl_silence(void); void mpu_all_notes_off(void); void opl_keys_off(void);
static void hush(void) { opl_silence(); mpu_all_notes_off(); }   /* (no note left hanging from the driver let go) */
static void soft_hush(void) { opl_keys_off(); mpu_all_notes_off(); }   /* (the same driver going on: every key let go, its instruments kept) */

static const char *const DEV[] = { "mt32", "fm", "adlib", "speaker", "midi" };
#define NDEVS 5
static int active, what, step, prev;     /* a preview under way: music (0) or battle sounds (1), its step, the piece before */
static u32 t0;
/* the battle sounds: ten seconds of a real battle as heard, each sound at its time (battle_sound(n)
   logged with DL_SFX_LOG=1, the game at its own pace, Improved combat on): a melee against six bandits,
   or one against archers and bandits as the shooting turns to blows; one of the two at random. n: 10 a
   death, 15 a bow or crossbow, 16 a knock-back, 17 a blow that misses or is parried (hits make no sound
   in the original) */
typedef struct { short ms; signed char n; } Beat;
static const Beat MELEE[] = {
    { 0, 16 }, { 0, 16 }, { 140, 17 }, { 280, 17 }, { 280, 17 }, { 280, 16 }, { 950, 17 }, { 1080, 17 },
    { 1220, 17 }, { 1510, 17 }, { 1510, 17 }, { 1630, 17 }, { 2050, 16 }, { 2050, 17 }, { 2350, 16 }, { 2350, 17 },
    { 2500, 17 }, { 2920, 16 }, { 3390, 17 }, { 3540, 16 }, { 3540, 17 }, { 3670, 17 }, { 3670, 10 }, { 3800, 17 },
    { 3800, 17 }, { 3930, 17 }, { 3930, 16 }, { 4780, 17 }, { 4780, 17 }, { 4780, 17 }, { 4910, 17 }, { 4910, 16 },
    { 5170, 17 }, { 5430, 17 }, { 5840, 17 }, { 5970, 17 }, { 6100, 17 }, { 6100, 17 }, { 6100, 17 }, { 6630, 17 },
    { 6910, 17 }, { 7030, 17 }, { 7170, 17 }, { 7170, 17 }, { 7170, 16 }, { 7170, 17 }, { 7460, 17 }, { 8020, 16 },
    { 8530, 17 }, { 8680, 17 }, { 8680, 17 }, { 8680, 17 }, { 8680, 17 }, { 8800, 17 }, { 9330, 17 }, { 9610, 17 },
    { 9870, 17 }, { 9870, 17 }, { 9870, 17 }
};
static const Beat ARCHERS[] = {
    { 0, 17 }, { 0, 17 }, { 130, 15 }, { 130, 17 }, { 420, 17 }, { 670, 17 }, { 1480, 17 }, { 1610, 17 },
    { 1610, 17 }, { 1750, 16 }, { 1880, 17 }, { 2160, 16 }, { 2950, 17 }, { 3350, 17 }, { 3350, 17 }, { 3750, 17 },
    { 3880, 17 }, { 3880, 16 }, { 4420, 17 }, { 4700, 17 }, { 4830, 17 }, { 4830, 17 }, { 5130, 17 }, { 5260, 17 },
    { 5260, 16 }, { 5510, 16 }, { 5860, 16 }, { 5870, 17 }, { 6240, 16 }, { 6420, 17 }, { 6590, 17 }, { 6590, 17 },
    { 7090, 17 }, { 7500, 16 }, { 7500, 17 }, { 7720, 10 }, { 7920, 17 }, { 7920, 17 }, { 7920, 17 }, { 8120, 17 },
    { 8120, 17 }, { 8120, 17 }, { 8280, 17 }, { 8430, 17 }, { 8790, 16 }, { 8980, 17 }, { 8980, 17 }, { 8980, 16 },
    { 8990, 17 }, { 9140, 17 }, { 9280, 17 }, { 9280, 17 }, { 9280, 16 }, { 9710, 17 }
};
static int seg;                          /* (which of the two) */

int musicpack_now(int *from_pack);
void musicpack_hold(int on);
static int music_was = -1;               /* the game's own music switch (Alt+M), on for a preview of the music, as it was after */
static void begin(int dev, int w, int c, int d)
{
    (void)d;
    if (game_mode() == GM_BATTLE || dev < 0 || dev >= NDEVS || !g_rw(0x9417)) return;
    int fp; prev = musicpack_now(&fp);
    snprintf(over, sizeof over, "%s", DEV[dev]);
    musicpack_hold(1);
    play(1); hush();
    load_driver(w == 1);                 /* (the battle sounds: the battles' driver; the music: the towns') */
    active = 1; what = w; step = 0; t0 = plat_ticks_ms(); seg = rand() & 1;
    if (w != 1 && music_was < 0) { music_was = g_rb(GV_opt_music); g_wb(GV_opt_music, 1); }
    if (!w) play(20);                      /* the second tune of the overland view: every device has it */
    if (w == 2) { play(c); void playlist_started(int); playlist_started(c); }   /* (the soundtrack's player: piece c, until stopped) */
    fprintf(stderr, "sound preview: %s, %s\n", DEV[dev], w == 1 ? "battle sounds" : w == 2 ? "the soundtrack" : "music");
}
static void end(int a, int b, int c, int d)
{
    (void)a; (void)b; (void)c; (void)d;
    if (!active) return;
    active = 0;
    play(1); hush();
    over[0] = 0;
    if (music_was >= 0) { g_wb(GV_opt_music, (u8)music_was); music_was = -1; }
    load_driver(0);
    musicpack_hold(0);
    if (prev >= 8) play(prev);
    fprintf(stderr, "sound preview: done\n");
}
static void beat(int n, int b, int c, int d) { (void)b; (void)c; (void)d; if (active) play(n); }

/* the soundtrack's player (mods/playlist.c): piece n through device dev until stopped; within a
   deferred call, a piece started again from its beginning, the notes of the last one silenced */
void soundpreview_piece(const char *dev, int n)
{
    int k = -1; for (int i = 0; i < NDEVS; i++) if (!strcmp(dev, DEV[i])) k = i;
    if (k < 0) return;
    if (active) ns_defer(end, 0, 0, 0, 0);
    ns_defer(begin, k, 2, n, 0);
}
void soundpreview_replay(int n) { if (active) { play(1); soft_hush(); play(n); } }
void soundpreview_hush(void) { soft_hush(); }
int soundpreview_device(void) { for (int i = 0; i < NDEVS; i++) if (active && !strcmp(over, DEV[i])) return i; return -1; }

/* the settings' buttons: hear device dev ("mt32", "fm" ...) play music (0) or battle sounds (1) */
void soundpreview_start(const char *dev, int w)
{
    int k = -1; for (int i = 0; i < NDEVS; i++) if (!strcmp(dev, DEV[i])) k = i;
    if (k < 0) return;
    if (active) ns_defer(end, 0, 0, 0, 0);
    ns_defer(begin, k, w, 0, 0);
}
void soundpreview_stop(void) { if (active) ns_defer(end, 0, 0, 0, 0); }
int soundpreview_playing(const char *dev, int w) { return active && over[0] && !strcmp(over, dev) && what == w; }
int soundpreview_active(void) { return active != 0; }
int soundpreview_possible(void) { return game_mode() != GM_BATTLE && game_mode() != GM_OTHER; }
/* each frame (ui/ui.cpp): the battle sounds one after another; the end after a while */
void soundpreview_tick(void)
{
    if (!active) return;
    u32 dt = plat_ticks_ms() - t0;
    if (what == 1) {
        const Beat *b = seg ? ARCHERS : MELEE; int n = seg ? (int)(sizeof ARCHERS / sizeof ARCHERS[0]) : (int)(sizeof MELEE / sizeof MELEE[0]);
        for (int q = 0; step < n && dt >= (u32)(300 + b[step].ms) && q < 6; q++) { ns_defer(beat, b[step].n, 0, 0, 0); step++; }   /* (as many as are due, a few a frame) */
        if (step >= n && dt > 300u + 10000u + 900u) { ns_defer(end, 0, 0, 0, 0); active = 2; }
    } else if (what == 0 && dt > 15000 && active == 1) { ns_defer(end, 0, 0, 0, 0); active = 2; }   /* (the soundtrack's player plays until stopped) */
}
