/* mpu.c - MPU-401 MIDI interface (ports 330h/331h). The game's music driver (the same
 * *SOUND.DLC for every card in this release) is a Roland MPU-401 driver; its MIDI stream goes
 * to the host's MIDI synthesizer (Windows: the system MIDI device, as DOSBox does). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "hle.h"
#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
static HMIDIOUT hmo;
#endif

static u8 inq[16]; static int inq_n;
static u8 msg[3]; static int msg_len, msg_need;
static u8 running;
static u8 sysex[512]; static int sysex_n, in_sysex;
static int midi_ok = -1;
static FILE *midi_log;

static void midi_open(void)
{
    midi_ok = 0;
    if (getenv("DL_NOSOUND")) return;
    const char *lg = getenv("DL_MIDI_LOG");
    if (lg) midi_log = fopen(lg, "wb");
#ifdef _WIN32
    if (midiOutOpen(&hmo, MIDI_MAPPER, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR) midi_ok = 1;
    else fprintf(stderr, "no MIDI output device\n");
#endif
}

static void midi_short(u8 a, u8 b, u8 c, int n)
{
    if (midi_log) { u8 m[3] = {a, b, c}; fwrite(m, 1, (size_t)n, midi_log); }
#ifdef _WIN32
    if (midi_ok > 0) midiOutShortMsg(hmo, (DWORD)a | ((DWORD)b << 8) | ((DWORD)c << 16));
#endif
    (void)n;
}

static void midi_sysex(const u8 *d, int n)
{
    if (midi_log) fwrite(d, 1, (size_t)n, midi_log);
#ifdef _WIN32
    if (midi_ok > 0) {
        MIDIHDR h; memset(&h, 0, sizeof h);
        h.lpData = (LPSTR)d; h.dwBufferLength = (DWORD)n;
        if (midiOutPrepareHeader(hmo, &h, sizeof h) == MMSYSERR_NOERROR) {
            midiOutLongMsg(hmo, &h, sizeof h);
            for (int i = 0; i < 100 && midiOutUnprepareHeader(hmo, &h, sizeof h) == MIDIERR_STILLPLAYING; i++) Sleep(1);
        }
    }
#endif
}

/* the game held (menu, world map): no notes may hang on in the synthesizer */
void mpu_all_notes_off(void)
{
    if (midi_ok <= 0) return;
    for (int ch = 0; ch < 16; ch++) { midi_short((u8)(0xb0 | ch), 123, 0, 3); midi_short((u8)(0xb0 | ch), 64, 0, 3); }
}

static int data_len(u8 st)
{
    switch (st & 0xf0) {
    case 0xc0: case 0xd0: return 1;
    case 0xf0: return st == 0xf1 || st == 0xf3 ? 1 : st == 0xf2 ? 2 : 0;
    default: return 2;
    }
}

static void midi_byte(u8 v)
{
    if (midi_ok < 0) midi_open();
    if (in_sysex) {
        if (sysex_n < (int)sizeof sysex) sysex[sysex_n++] = v;
        if (v == 0xf7) { in_sysex = 0; midi_sysex(sysex, sysex_n); }
        return;
    }
    if (v == 0xf0) { in_sysex = 1; sysex_n = 0; sysex[sysex_n++] = v; return; }
    if (v >= 0xf8) { midi_short(v, 0, 0, 1); return; }     /* real time */
    if (v & 0x80) {
        running = v; msg[0] = v; msg_len = 1; msg_need = data_len(v);
        if (!msg_need) { midi_short(v, 0, 0, 1); msg_len = 0; }
        return;
    }
    if (!msg_len) {                                          /* running status */
        if (!running) return;
        msg[0] = running; msg_len = 1; msg_need = data_len(running);
    }
    msg[msg_len++] = v;
    if (msg_len == msg_need + 1) { midi_short(msg[0], msg[1], msg_len > 2 ? msg[2] : 0, msg_len); msg_len = 0; }
}

void mpu_out(u16 port, u8 v)
{
    if (port & 1) {            /* command: reset, UART mode, ... - all acknowledged */
        if (v == 0xff) { inq_n = 0; in_sysex = 0; msg_len = 0; running = 0; }
        if (inq_n < 16) inq[inq_n++] = 0xfe;
        return;
    }
    midi_byte(v);
}

u8 mpu_in(u16 port)
{
    if (port & 1) return (u8)(inq_n ? 0x00 : 0x80);   /* bit 6 clear: ready; bit 7 clear: data */
    if (!inq_n) return 0xfe;
    u8 v = inq[0];
    memmove(inq, inq + 1, (size_t)--inq_n);
    return v;
}
