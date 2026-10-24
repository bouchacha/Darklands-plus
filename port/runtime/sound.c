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
#include "../mods/musicpack.h"

static opl3_chip chip;
static SDL_AudioDeviceID dev;
static int opl_index[2];
static int ok;
/* timers (status register) */
static uint8_t t1_val, t2_val, t_ctrl, status;
static uint64_t t1_start, t2_start;

static FILE *wav; static uint32_t wav_bytes;
int sound_music_volume = 100;                                /* percent, F12 > Settings (sound.music_volume) */
/* pause (the game held while the menu or the world map is open): the chip, the sample
 * channel and the effects stop where they are, after a short fade, and go on after it */
volatile int sound_music_hushed;                          /* the music quiet while the settings are open (but a preview) */
static volatile int paused; static int fade;              /* fade: 0 none, 1 out, 2 in */
static void callback(void *u, Uint8 *stream, int len)
{
    (void)u;
    int16_t *s16 = (int16_t *)stream; int n = len / 4;
    if (paused && !fade) { memset(stream, 0, (size_t)len); return; }
    OPL3_GenerateStream(&chip, s16, (uint32_t)n);
    { void mt32_mix(short *, int); mt32_mix((short *)stream, n); }
    if (musicpack_mutes() || sound_music_hushed) memset(stream, 0, (size_t)len);   /* a music pack's piece plays instead; or the settings are open */
    else if (sound_music_volume != 100)                      /* the music (FM or MT-32), before the effects go in */
        for (int i = 0; i < 2 * n; i++) {
            int y = s16[i] * sound_music_volume / 100;
            s16[i] = (int16_t)(y > 32767 ? 32767 : y < -32768 ? -32768 : y);
        }
    if (!sound_music_hushed) musicpack_mix((short *)stream, n);
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

static uint32_t chip_rate = 49716;
void sound_init(void)
{
    { int config_get_int(const char *, int); sound_music_volume = config_get_int("sound.music_volume", 100); }
    { void sb_init(void); sb_init(); }
    if (getenv("DL_NOSOUND")) return;
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) { fprintf(stderr, "no audio: %s\n", SDL_GetError()); return; }
    SDL_AudioSpec want = {0}, have;
    want.freq = 44100; want.format = AUDIO_S16SYS; want.channels = 2; want.samples = 1024; want.callback = callback;
    dev = SDL_OpenAudioDevice(0, 0, &want, &have, 0);
    if (!dev) { fprintf(stderr, "no audio device: %s\n", SDL_GetError()); return; }
    OPL3_Reset(&chip, (uint32_t)have.freq); chip_rate = (uint32_t)have.freq;
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
int opl_write_count(void) { return nwrites; }
static void stats(void) { if (getenv("DL_TRACE_OPL")) fprintf(stderr, "opl: %d writes, %d status reads\n", nwrites, nreads); }
/* the FM chip silenced at once (a note held when its driver was let go would sound on for ever):
   the sound preview (mods/soundpreview.c) changes drivers in the middle of the music */
void opl_silence(void)
{
    if (!ok) return;
    SDL_LockAudioDevice(dev);
    OPL3_Reset(&chip, chip_rate);
    SDL_UnlockAudioDevice(dev);
}
extern int music_ff;
void playlist_heard(int key);
static u8 opl_keys[2][9], opl_fnum[2][9];        /* the key-on registers last written (0xb0 ...), for opl_keys_off; the frequencies' low bytes (0xa0 ...) */
void opl_keys_off(void)                          /* every channel's key released (the instruments kept, as a reset would not) */
{
    if (!ok) return;
    SDL_LockAudioDevice(dev);
    for (int b = 0; b < 2; b++) for (int ch = 0; ch < 9; ch++) OPL3_WriteRegBuffered(&chip, (uint16_t)((0xb0 + ch) | (b << 8)), opl_keys[b][ch] & ~0x20);
    SDL_UnlockAudioDevice(dev);
}
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
    if (reg >= 0xa0 && reg <= 0xa8) opl_fnum[bank][reg - 0xa0] = v;
    if (reg >= 0xb0 && reg <= 0xb8) {
        if (music_ff) { if ((v & 0x20) && !(opl_keys[bank][reg - 0xb0] & 0x20)) playlist_heard(0x10000 | (v & 0x1f) << 8 | opl_fnum[bank][reg - 0xb0]); v &= ~0x20; }   /* (moving through a piece: no key sounds; its pitch noted, whatever the channel) */
        opl_keys[bank][reg - 0xb0] = v;
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
