/* speaker.c - the PC speaker, which the port had not made at all ("PC speaker" in F12 > Settings >
 * Sound played nothing). The game's speaker driver (ISOUND.DLC / .DLB, "DarkCardsIBM") plays its
 * tunes the usual way: the timer's channel 2 set to a note (port 43h: channel 2, then its divisor
 * through port 42h) and the speaker's two gate bits in port 61h; both bits set, the speaker sounds
 * a square wave at 1193182 / divisor. Made here in the audio callback (runtime/sound.c), each
 * change at its moment one buffer late, as the MT-32's messages are (runtime/mt32.c): an even
 * delay, no notes bunched at the start of a buffer. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cpu.h"

uint64_t plat_ticks_us(void);
void sound_lock(int on);

static uint16_t div2;                    /* channel 2's divisor (0: 65536) */
static int access2 = 3, hi_next;         /* how channel 2 takes its divisor: 1 low byte, 2 high byte, 3 low then high */
static uint8_t p61;                      /* port 61h as last written */

typedef struct { uint32_t pos; uint16_t div; uint8_t on; } Ev;
#define NEV 512
static Ev ev[NEV];
static unsigned ev_r, ev_w;
static uint64_t cb_us;                   /* when the last buffer was made */
static uint32_t rendered, cb_frames = 1024;
static int cur_on; static uint16_t cur_div; static double phase;
static int trace = -1;

static void change(void)
{
    if (trace < 0) trace = getenv("DL_TRACE_SPEAKER") != 0;
    int on = (p61 & 3) == 3;
    if (trace) fprintf(stderr, "speaker: %s, divisor %u (%.0f Hz)\n", on ? "on" : "off", div2, 1193182.0 / (div2 ? div2 : 65536));
    sound_lock(1);
    extern volatile uint32_t sound_tick_late_us;           /* (runtime/bios.c: dated back by a late timer tick, as the music) */
    int64_t dt = (int64_t)(plat_ticks_us() - cb_us) - (int64_t)sound_tick_late_us + 8000;
    if (dt < 0) dt = 0;
    if (dt > 2 * 1000000ll * cb_frames / 44100) dt = 2 * 1000000ll * cb_frames / 44100;   /* (after a pause) */
    if (ev_w - ev_r < NEV) { Ev *e = &ev[ev_w % NEV]; e->pos = rendered + (uint32_t)(dt * 44100 / 1000000); e->div = div2; e->on = (uint8_t)on; ev_w++; }
    sound_lock(0);
}

/* the ports (runtime/bios.c): 42h, 43h (channel 2's control words only; channel 0's are the
   timer's) and 61h */
void spk_out(u16 port, u8 v)
{
    if (trace < 0) trace = getenv("DL_TRACE_SPEAKER") != 0;
    if (trace > 0) { static unsigned n; if (n++ < 200) fprintf(stderr, "speaker: out %02x %02x\n", port, v); }
    if (port == 0x43) { if ((v >> 6) == 2 && (v & 0x30)) { access2 = (v >> 4) & 3; hi_next = 0; } return; }
    if (port == 0x42) {
        if (access2 == 1) div2 = (uint16_t)((div2 & 0xff00) | v);
        else if (access2 == 2) div2 = (uint16_t)((div2 & 0x00ff) | v << 8);
        else if (!hi_next) { div2 = (uint16_t)((div2 & 0xff00) | v); hi_next = 1; return; }   /* (the note changes with its high byte) */
        else { div2 = (uint16_t)((div2 & 0x00ff) | v << 8); hi_next = 0; }
        if ((p61 & 3) == 3) change();
        return;
    }
    if (port == 0x61) { uint8_t was = p61; p61 = v; if ((was ^ v) & 3) change(); }
}
u8 spk_in61(void) { return (u8)((p61 & 0x0f) | 0x20); }

/* from the audio callback: the speaker's note added to the stream (before the music's volume) */
void spk_mix(int16_t *s, int frames)
{
    cb_us = plat_ticks_us(); cb_frames = (uint32_t)frames;
    if (ev_r == ev_w && !cur_on) { rendered += (uint32_t)frames; return; }
    for (int i = 0; i < frames; i++) {
        while (ev_r != ev_w && (int32_t)(ev[ev_r % NEV].pos - (rendered + (uint32_t)i)) <= 0) { cur_on = ev[ev_r % NEV].on; cur_div = ev[ev_r % NEV].div; ev_r++; }
        if (!cur_on) continue;
        double f = 1193182.0 / (cur_div ? cur_div : 65536);
        if (f > 16000) continue;                                  /* (above hearing: silence, as a speaker would give) */
        phase += f / 44100.0; phase -= floor(phase);
        int a = phase < 0.5 ? 2600 : -2600;
        for (int c = 0; c < 2; c++) { int y = s[2 * i + c] + a; s[2 * i + c] = (int16_t)(y > 32767 ? 32767 : y < -32768 ? -32768 : y); }
    }
    rendered += (uint32_t)frames;
}
