/* mpu.c - MPU-401 MIDI interface (ports 330h/331h). The game's Roland driver (RSOUND.DLC /
 * .DLB) talks to an MPU-401; its MIDI stream goes to the emulated MT-32 (mt32.c) when that is
 * chosen and its ROMs are there, otherwise to the host's MIDI synthesizer (Windows: the system
 * MIDI device, as DOSBox does). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "hle.h"
#include "config.h"
#include "../mods/musicpack.h"
#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
static HMIDIOUT hmo;
#endif

static u8 inq[16]; static int inq_n;
static u8 msg[3]; static int msg_len, msg_need;
static u8 running;
static u8 sysex[512]; static int sysex_n, in_sysex;
static int midi_ok = -1;                /* -1 not opened yet, 0 nowhere, 1 the system's MIDI device, 2 the emulated MT-32 */
static FILE *midi_log;
int mt32_open(int rate);
void mt32_msg(const unsigned char *m, int n);
int sound_use_mt32(void);

static void midi_open(void)
{
    midi_ok = 0;
    if (getenv("DL_NOSOUND")) return;
    const char *lg = getenv("DL_MIDI_LOG");
    if (lg && !midi_log && (midi_log = fopen(lg, "wb"))) setvbuf(midi_log, NULL, _IONBF, 0);
    if (sound_use_mt32() && mt32_open(44100)) { midi_ok = 2; return; }
#ifdef _WIN32
    /* the output named in F12 > Settings > Display (sound.midi_out; a real MT-32 on a MIDI interface, a
     * software synthesizer ...), else Windows' default */
    static int win = -1; static char opened[64];
    const char *want = config_get("sound.midi_out", "");
    if (win > 0 && strcmp(opened, want)) { midiOutReset(hmo); midiOutClose(hmo); win = -1; }
    if (win < 0) {
        UINT id = MIDI_MAPPER;
        for (UINT i = 0; want[0] && i < midiOutGetNumDevs(); i++) {
            MIDIOUTCAPSA c;
            if (midiOutGetDevCapsA(i, &c, sizeof c) == MMSYSERR_NOERROR && !strcmp(c.szPname, want)) id = i;
        }
        win = midiOutOpen(&hmo, id, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR;
        snprintf(opened, sizeof opened, "%s", want);
        if (!win) fprintf(stderr, "no MIDI output device\n");
        else fprintf(stderr, "MIDI out: %s\n", id == MIDI_MAPPER ? "the system's default" : want);
    }
    midi_ok = win;
#endif
}
/* a driver is being loaded (sounddev.c): where the MIDI goes is decided again at its first byte */
void mpu_reconsider(void) { midi_ok = -1; }

/* the system's MIDI outputs, for F12 > Settings > Display */
int mpu_out_count(void)
{
#ifdef _WIN32
    return (int)midiOutGetNumDevs();
#else
    return 0;
#endif
}
const char *mpu_out_name(int i)
{
#ifdef _WIN32
    static char n[64]; MIDIOUTCAPSA c;
    if (midiOutGetDevCapsA((UINT)i, &c, sizeof c) != MMSYSERR_NOERROR) return "";
    snprintf(n, sizeof n, "%s", c.szPname);
    return n;
#else
    (void)i; return "";
#endif
}

static void midi_short(u8 a, u8 b, u8 c, int n)
{
    if (midi_log) { u8 m[3] = {a, b, c}; fwrite(m, 1, (size_t)n, midi_log); }
    if (midi_ok == 2) { u8 m[3] = {a, b, c}; mt32_msg(m, n); return; }
#ifdef _WIN32
    if (midi_ok == 1 && !((a & 0xf0) == 0x90 && c && musicpack_mutes()))     /* no notes while a music pack's piece plays */
        midiOutShortMsg(hmo, (DWORD)a | ((DWORD)b << 8) | ((DWORD)c << 16));
#endif
    (void)n;
}

static void midi_sysex(const u8 *d, int n)
{
    if (midi_log) fwrite(d, 1, (size_t)n, midi_log);
    if (midi_ok == 2) { mt32_msg(d, n); return; }
#ifdef _WIN32
    if (midi_ok == 1) {
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
