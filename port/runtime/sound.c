/* sound.c - AdLib / OPL music through an emulated OPL chip (Nuked OPL3) on SDL audio.
 * The game's own AdLib driver (ASOUND.DLC, recompiled) programs the chip through ports
 * 388h/389h exactly as on the real card; this file is the card. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "cpu.h"
#include "hle.h"
#include "../third_party/nuked-opl3/opl3.h"

static opl3_chip chip;
static SDL_AudioDeviceID dev;
static int opl_index[2];
static int ok;
/* timers (status register) */
static uint8_t t1_val, t2_val, t_ctrl, status;
static uint64_t t1_start, t2_start;

static FILE *wav; static uint32_t wav_bytes;
/* pause (the game held while the menu or the world map is open): the chip, the sample
 * channel and the effects stop where they are, after a short fade, and go on after it */
static volatile int paused; static int fade;              /* fade: 0 none, 1 out, 2 in */
static void callback(void *u, Uint8 *stream, int len)
{
    (void)u;
    int16_t *s16 = (int16_t *)stream; int n = len / 4;
    if (paused && !fade) { memset(stream, 0, (size_t)len); return; }
    OPL3_GenerateStream(&chip, s16, (uint32_t)n);
    { void sb_mix(int16_t *, int, int); sb_mix(s16, n, 44100); }
    { void sfx_mix(short *, int); sfx_mix((short *)stream, n); }
    if (fade) {
        for (int i = 0; i < n; i++) {
            int g = fade == 1 ? (n - i) * 256 / n : i * 256 / n;
            s16[2 * i] = (int16_t)(s16[2 * i] * g / 256); s16[2 * i + 1] = (int16_t)(s16[2 * i + 1] * g / 256);
        }
        fade = 0;
    }
    if (wav) { fwrite(stream, 1, (size_t)len, wav); wav_bytes += (uint32_t)len; }
}
void sound_pause(int on)
{
    if (!dev || on == paused) return;
    SDL_LockAudioDevice(dev);
    paused = on; fade = on ? 1 : 2;
    SDL_UnlockAudioDevice(dev);
    { void mpu_all_notes_off(void); if (on) mpu_all_notes_off(); }
}
static void wav_close(void)
{
    if (!wav) return;
    uint32_t h[11] = { 0x46464952, 36 + wav_bytes, 0x45564157, 0x20746d66, 16, 0x00020001, 44100, 44100 * 4, 0x00100004, 0x61746164, wav_bytes };
    fseek(wav, 0, SEEK_SET); fwrite(h, 4, 11, wav); fclose(wav); wav = 0;
}

void sound_init(void)
{
    { void sb_init(void); sb_init(); }
    if (getenv("DL_NOSOUND")) return;
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) { fprintf(stderr, "no audio: %s\n", SDL_GetError()); return; }
    SDL_AudioSpec want = {0}, have;
    want.freq = 44100; want.format = AUDIO_S16SYS; want.channels = 2; want.samples = 1024; want.callback = callback;
    dev = SDL_OpenAudioDevice(0, 0, &want, &have, 0);
    if (!dev) { fprintf(stderr, "no audio device: %s\n", SDL_GetError()); return; }
    OPL3_Reset(&chip, (uint32_t)have.freq);
    { void sb_set_audio(SDL_AudioDeviceID); sb_set_audio(dev); }
    const char *w = getenv("DL_WAV");        /* debugging: record the music */
    if (w && (wav = fopen(w, "wb"))) { fseek(wav, 44, SEEK_SET); atexit(wav_close); }
    SDL_PauseAudioDevice(dev, 0);
    ok = 1;
}

void sound_lock(int on) { if (!dev) return; if (on) SDL_LockAudioDevice(dev); else SDL_UnlockAudioDevice(dev); }

static void update_timers(void)
{
    uint64_t now = plat_ticks_us();
    if ((t_ctrl & 1) && !(t_ctrl & 0x40)) {
        uint64_t per = 80ull * (256 - t1_val);
        if (now - t1_start >= per) status |= 0xc0;
    }
    if ((t_ctrl & 2) && !(t_ctrl & 0x20)) {
        uint64_t per = 320ull * (256 - t2_val);
        if (now - t2_start >= per) status |= 0xa0;
    }
}

static int nwrites, nreads;
static void stats(void) { if (getenv("DL_TRACE_OPL")) fprintf(stderr, "opl: %d writes, %d status reads\n", nwrites, nreads); }
void opl_out(u16 port, u8 v)
{
    if (!nwrites++) atexit(stats);
    if (getenv("DL_TRACE_OPL") && nwrites < 40) fprintf(stderr, "opl out %03x %02x\n", port, v);
    int bank = (port & 2) ? 1 : 0;
    if (!(port & 1)) { opl_index[bank] = v; return; }
    int reg = opl_index[bank];
    if (!bank && reg == 2) { t1_val = v; return; }
    if (!bank && reg == 3) { t2_val = v; return; }
    if (!bank && reg == 4) {
        if (v & 0x80) { status = 0; return; }
        uint64_t now = plat_ticks_us();
        if ((v & 1) && !(t_ctrl & 1)) t1_start = now;
        if ((v & 2) && !(t_ctrl & 2)) t2_start = now;
        t_ctrl = v;
        return;
    }
    if (!ok) return;
    SDL_LockAudioDevice(dev);
    OPL3_WriteRegBuffered(&chip, (uint16_t)(reg | (bank << 8)), v);
    SDL_UnlockAudioDevice(dev);
}

u8 opl_in(u16 port)
{
    (void)port;
    nreads++;
    update_timers();
    if (getenv("DL_TRACE_OPL") && nreads < 40) fprintf(stderr, "opl in %03x -> %02x\n", port, status | 6);
    return (u8)(status | 0x06);
}
