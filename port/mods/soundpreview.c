/* soundpreview.c - hearing a sound device before choosing it (F12 > Settings > Window, sound and saves):
 * a piece of music, or a few battle sounds, played through that device now, then the chosen one
 * back. The game's own way of changing its sound driver (as it does going into a battle and out,
 * s002 0e76): stop it (047c:0791, 00a9), let go of it (08f9), load xSOUND.DLC or .DLB (07ce: the
 * driver file is picked by the device, runtime/sounddev.c, which reads the device to hear from
 * here meanwhile), take it in (08c8), set it up (0783 with the card's port, IRQ, DMA) and start it
 * (0066); then sound_play(n). Not during a battle (its driver is busy); the music pack is held
 * back meanwhile, so the device itself is heard. Also the packs' own previews (a recording from
 * the music pack, the battle sounds pack). */
#include <stdio.h>
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

static const char *const DEV[] = { "mt32", "fm", "adlib", "speaker", "midi" };
#define NDEVS 5
static int active, what, step, prev;     /* a preview under way: music (0) or battle sounds (1), its step, the piece before */
static u32 t0;
static const int BATTLE[] = { 17, 16, 17, 15, 11, 10 };   /* a miss or parry, a knock-back, another, a bow, a gun, a death */

int musicpack_now(int *from_pack);
void musicpack_hold(int on);
static void begin(int dev, int w, int c, int d)
{
    (void)c; (void)d;
    if (game_mode() == GM_BATTLE || dev < 0 || dev >= NDEVS || !g_rw(0x9417)) return;
    int fp; prev = musicpack_now(&fp);
    snprintf(over, sizeof over, "%s", DEV[dev]);
    musicpack_hold(1);
    play(1);
    load_driver(w);
    active = 1; what = w; step = 0; t0 = plat_ticks_ms();
    if (!w) play(11);                      /* the market's tune: lively, every device has it */
    fprintf(stderr, "sound preview: %s, %s\n", DEV[dev], w ? "battle sounds" : "music");
}
static void end(int a, int b, int c, int d)
{
    (void)a; (void)b; (void)c; (void)d;
    if (!active) return;
    active = 0;
    play(1);
    over[0] = 0;
    load_driver(0);
    musicpack_hold(0);
    if (prev >= 8) play(prev);
    fprintf(stderr, "sound preview: done\n");
}
static void beat(int n, int b, int c, int d) { (void)b; (void)c; (void)d; if (active) play(n); }

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
int soundpreview_possible(void) { return game_mode() != GM_BATTLE && game_mode() != GM_OTHER; }
/* each frame (ui/ui.cpp): the battle sounds one after another; the end after a while */
void soundpreview_tick(void)
{
    if (!active) return;
    u32 dt = plat_ticks_ms() - t0;
    if (what == 1) {
        int n = (int)(sizeof BATTLE / sizeof BATTLE[0]);
        if (step < n && dt >= (u32)(300 + step * 700)) { ns_defer(beat, BATTLE[step], 0, 0, 0); step++; }
        if (dt > (u32)(300 + n * 700 + 800)) { ns_defer(end, 0, 0, 0, 0); active = 2; }
    } else if (dt > 15000 && active == 1) { ns_defer(end, 0, 0, 0, 0); active = 2; }
}
