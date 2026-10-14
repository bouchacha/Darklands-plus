/* opening.c - the opening (the MicroProse logo, the animations and the digitized speech) is
 * skipped once it has been seen (F12 > Display > "Play the opening"; "Play the original game"
 * plays it).
 *
 * The game has its own switch for this: the command line option Q (D_0bf2, s000 startup_config)
 * skips the opening in the main loop (root 03a1:000a) but also the death and ending animations.
 * So D_0bf2 is set only on the way into the main loop and cleared at its first call after the
 * opening block (sub_root_0617_06d8("msgfiles"), the start of the game's loading). */
#include <stdio.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/config.h"

#define D_QUICK 0x0bf2          /* int: 'Q' on the command line */
static int set_by_us;

static void mainloop_hook(void)                 /* root 03a1:000a: the opening, then the game */
{
    int play = config_get_int("original", 0) || config_get_int("display.play_opening", 0) || !config_get_int("display.opening_seen", 0);
    if (!config_get_int("display.opening_seen", 0)) { config_set_int("display.opening_seen", 1); config_save(); }
    if (!play && !g_rw(D_QUICK)) { g_ww(D_QUICK, 1); set_by_us = 1; fprintf(stderr, "opening: skipped (F12 > Display plays it)\n"); }
    hook_call_original();
}

static void loading_hook(void)                  /* root 0617:06d8: the first call ends the opening block */
{
    if (set_by_us) { g_ww(D_QUICK, 0); set_by_us = 0; }
    hook_call_original();
}

void opening_init(void)
{
    hook_install(-1, 0x03a1, 0x000a, mainloop_hook);
    hook_install(-1, 0x0617, 0x06d8, loading_hook);
}
