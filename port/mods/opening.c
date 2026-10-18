/* opening.c - the opening (the MicroProse logo, the animations, the music and the digitized
 * speech), F12 > Settings > Display > "At start": the whole opening (the first start always),
 * its last scene alone (the lady of the Darklands and her welcome; the default), or nothing.
 * "Play the original game" plays it whole. Played whole (the default), a key or a click during the
 * gargoyles and the rest goes on to the lady's welcome (opening8 and 9), and one there ends it; the
 * last scene alone, or the original, a key ends it at once.
 *
 * The game has its own switch for skipping it: the command line option Q (D_0bf2, s000
 * startup_config) skips the opening in the main loop (root 03a1:000a) but also the death and
 * ending animations. So D_0bf2 is set only on the way into the main loop and cleared at its first
 * call after the opening block (sub_root_0617_06d8("msgfiles"), the start of the game's loading).
 *
 * The opening is seven PAN animations (opening2 ... opening9), each started by the main loop with
 * the music cue for it (vec_18f1 0x2f ... 0x35) and played by the PAN player (root 030b:0004): a
 * reader fills a ring buffer and returns at a key, then 030b:00fe waits until every cel read was
 * shown, calling 04fc (kbhit) meanwhile; the later animations' readers then return at once for
 * the key still waiting. So a key ended the opening only when the animation playing had shown
 * what was already read (most of it). Here 04fc, at a key, moves the head to the tail: what is
 * read counts as shown. The last scene alone: every animation but opening8 and 9 is passed over
 * (their music cues still follow one another, so opening8 and 9 get their own).
 *
 * The speech: with a Sound Blaster (CONFIG.DRK, as the GOG release has it) the main loop loads
 * OPENDARK.DGT into a buffer and gives it to the sound driver, and cues 0x34/0x35 play the lady's
 * words from it. After opening9 the buffer is freed, but the song of cue 0x35 goes on and plays
 * its words again from time to time - from memory that loading a game then fills with other
 * things: a loud screech every few seconds, the music cut, for the session. A key at the end
 * makes the game stop the song (vec_18f1(0), then the title's vec_18f1(7)); so when the opening
 * ends by itself, a key is given for it, and the game takes its own way. */
#include <stdio.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/hle.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"

#define D_QUICK 0x0bf2          /* int: 'Q' on the command line */
#define PAN_HEAD 0xa716         /* unsigned long: the next cel to show (the timer advances it) */
#define PAN_TAIL 0xa71a         /* unsigned long: the end of the last cel read */
static int set_by_us, end_only, to_welcome;   /* to_welcome: a key before the lady's scene: on to it */
static char cur_pan[16];
static int welcome_pan(const char *n) { return !strcmp(n, "opening8") || !strcmp(n, "opening9"); }
int opening_playing;            /* (platform_sdl.c: a click then ends it, as a key) */

int opening_mode(void)          /* 0 nothing, 1 the last scene, 2 the whole opening */
{
    int m = config_get_int("display.opening", -1);
    if (m < 0) m = 2;                                                        /* (the whole, a key on to the welcome) */
    return m < 0 ? 1 : m > 2 ? 2 : m;
}
void opening_set_mode(int m) { config_set_int("display.opening", m); }

static void mainloop_hook(void)                 /* root 03a1:000a: the opening, then the game */
{
    int mode = config_get_int("original", 0) || !config_get_int("display.opening_seen", 0) ? 2 : opening_mode();
    if (!config_get_int("display.opening_seen", 0)) { config_set_int("display.opening_seen", 1); config_save(); }
    end_only = mode == 1;
    if (mode == 0 && !g_rw(D_QUICK)) { g_ww(D_QUICK, 1); set_by_us = 1; fprintf(stderr, "opening: skipped (F12 > Settings > Display plays it)\n"); }
    else { opening_playing = 1; fprintf(stderr, "opening: %s\n", end_only ? "the last scene" : "whole"); }
    hook_call_original();
}

static void pan_hook(void)                      /* root 030b:0004 (char *name): play name.PAN */
{
    char name[16]; g_str(G_DGROUP, rw(SS, (u16)(SP + 4)), name, sizeof name);
    snprintf(cur_pan, sizeof cur_pan, "%s", name);
    int pass = opening_playing && (end_only || to_welcome) && !welcome_pan(name);
    if (opening_playing) fprintf(stderr, "opening: %s.pan%s\n", name, pass ? " passed over" : "");
    if (pass) { hook_return_far(0); return; }   /* (its music cue was given: the next follows) */
    if (opening_playing && to_welcome == 1 && welcome_pan(name)) { while (bios_kbd_avail()) bios_kbd_get(1); to_welcome = 2; }   /* (the key that brought us here) */
    hook_call_original();
    if (opening_playing && !strcmp(name, "opening9") && !bios_kbd_avail()) {   /* ended by itself: a key, so that the game stops the song */
        fprintf(stderr, "opening: ended by itself (a key for it: the song stops)\n");
        bios_kbd_inject(0x3920);
    }
}

static void tick_hook(void)                     /* root 030b:04fc: kbhit, while a PAN plays */
{
    hook_call_original();
    if (opening_playing && bios_kbd_avail() && !end_only && !to_welcome && !config_get_int("original", 0) && !welcome_pan(cur_pan)) {
        to_welcome = 1;                            /* a key before the lady's scene: this animation ends (the key stops its
                                                      reader), the rest is passed over, and the key is taken before hers */
        fprintf(stderr, "opening: on to the welcome\n");
    }
    if (opening_playing && bios_kbd_avail()) {   /* a key: what is read counts as shown */
        u32 tail = (u32)g_rw(PAN_TAIL) | (u32)g_rw(PAN_TAIL + 2) << 16;
        g_ww(PAN_HEAD, (u16)tail); g_ww(PAN_HEAD + 2, (u16)(tail >> 16));
    }
}

static void loading_hook(void)                  /* root 0617:06d8: the first call ends the opening block */
{
    if (set_by_us) { g_ww(D_QUICK, 0); set_by_us = 0; }
    opening_playing = 0; end_only = 0; to_welcome = 0;
    hook_call_original();
}

void opening_init(void)
{
    hook_install(-1, 0x03a1, 0x000a, mainloop_hook);
    hook_install(-1, 0x030b, 0x0004, pan_hook);
    hook_install(-1, 0x030b, 0x04fc, tick_hook);
    hook_install(GF_catalog_open, loading_hook);
}
