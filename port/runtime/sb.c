/* sb.c - the digital half of a Sound Blaster Pro: DSP (ports base+6/A/C/E), 8237 DMA
 * controller and the transfer-complete interrupt. Darklands' FM drivers (PSOUND/ASOUND .DC)
 * play the digitized effects of the opening and the endings (OPENDARK.DGT ...) through it.
 * The card's settings come from the game's CONFIG.DRK (written by its INSTALL program:
 * words: card, port, IRQ, DMA); default 220h, IRQ 5, DMA 1 as in the GOG release.
 *
 * Transfers are copied out of emulated memory when the DSP is told to start them, played
 * by the audio callback (sound.c mixes them over the FM chip), and their end is signalled
 * on the game's clock (dl_poll) when the block's duration has passed. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "cpu.h"
#include "hle.h"

extern char game_dir[1024];
int sb_base = 0x220, sb_irq = 5, sb_dma = 1;

/* ---- 8237 DMA (channels 0-3) */
static u16 dma_addr[4], dma_count[4];
static u8 dma_page[4], dma_mode[4], dma_ff, dma_mask = 0x0f;

void dma_out(u16 port, u8 v)
{
    if (port < 8) {
        int ch = port >> 1;
        u16 *r = (port & 1) ? &dma_count[ch] : &dma_addr[ch];
        if (!dma_ff) *r = (u16)((*r & 0xff00) | v); else *r = (u16)((*r & 0x00ff) | (v << 8));
        dma_ff ^= 1;
        return;
    }
    switch (port) {
    case 0x0a: if (v & 4) dma_mask |= (u8)(1 << (v & 3)); else dma_mask &= (u8)~(1 << (v & 3)); break;
    case 0x0b: dma_mode[v & 3] = v; break;
    case 0x0c: dma_ff = 0; break;
    case 0x0d: dma_mask = 0x0f; dma_ff = 0; break;           /* master clear */
    case 0x0f: dma_mask = v & 0x0f; break;
    case 0x87: dma_page[0] = v; break;
    case 0x83: dma_page[1] = v; break;
    case 0x81: dma_page[2] = v; break;
    case 0x82: dma_page[3] = v; break;
    }
}

u8 dma_in(u16 port)
{
    if (port < 8) {
        int ch = port >> 1;
        u16 r = (port & 1) ? dma_count[ch] : dma_addr[ch];
        u8 v = dma_ff ? (u8)(r >> 8) : (u8)r;
        dma_ff ^= 1;
        return v;
    }
    if (port == 0x08) return 0;
    return 0xff;
}

/* ---- DSP */
static u8 outq[16]; static int outq_n;            /* bytes for the game to read (22A) */
static u8 cmd, args[4]; static int nargs, want;
static int rate = 11025, block = 0x800, autoinit, playing, speaker = 1;
static uint64_t end_us;                                 /* when the current block is done */
static int irq_pending;
static u8 mixer_idx, mixer[256];

/* PCM ring for the audio thread: unsigned 8-bit samples at `rate` */
#define RING (1 << 18)
static u8 ring[RING]; static volatile u32 ring_w, ring_r;
static volatile int ring_rate = 11025;
static double frac;
static SDL_AudioDeviceID audio_dev;
void sb_set_audio(SDL_AudioDeviceID d) { audio_dev = d; }

static void push_bytes(u32 lin, u32 n)
{
    if (audio_dev) SDL_LockAudioDevice(audio_dev);
    ring_rate = rate;
    for (u32 i = 0; i < n; i++) {
        if (ring_w - ring_r >= RING) break;
        ring[ring_w++ & (RING - 1)] = speaker ? M[(lin + i) & 0xfffff] : 0x80;
    }
    if (audio_dev) SDL_UnlockAudioDevice(audio_dev);
}

/* audio thread: add the digital channel to the FM output */
void sb_mix(int16_t *out, int frames, int out_rate)
{
    double step = (double)ring_rate / out_rate;
    for (int i = 0; i < frames; i++) {
        if (ring_r == ring_w) break;
        int s = ((int)ring[ring_r & (RING - 1)] - 128) * 160;
        int l = out[2 * i] + s, r = out[2 * i + 1] + s;
        out[2 * i] = (int16_t)(l > 32767 ? 32767 : l < -32768 ? -32768 : l);
        out[2 * i + 1] = (int16_t)(r > 32767 ? 32767 : r < -32768 ? -32768 : r);
        frac += step;
        while (frac >= 1.0 && ring_r != ring_w) { ring_r++; frac -= 1.0; }
    }
}

static u32 dma_lin(int ch) { return ((u32)dma_page[ch] << 16) + dma_addr[ch]; }

static void start_block(int len)
{
    int ch = sb_dma & 3;
    u32 total = (u32)dma_count[ch] + 1;
    u32 n = (u32)len < total ? (u32)len : total;
    push_bytes(dma_lin(ch), n);
    if (!(dma_mode[ch] & 0x10)) {                  /* not auto-init: the controller advances */
        dma_addr[ch] = (u16)(dma_addr[ch] + n);
        dma_count[ch] = (u16)(dma_count[ch] - n);
    }
    playing = 1;
    end_us = plat_ticks_us() + (uint64_t)n * 1000000u / (uint64_t)(rate ? rate : 11025);
    if (getenv("DL_TRACE_SB")) fprintf(stderr, "sb: block of %u bytes at %05x, %d Hz%s\n", n, dma_lin(ch), rate, autoinit ? " (auto)" : "");
}

static void command(void)
{
    switch (cmd) {
    case 0x10: push_bytes(0, 0); break;                          /* direct DAC: ignored */
    case 0x14: case 0x91: autoinit = 0; start_block((args[0] | args[1] << 8) + 1); break;
    case 0x1c: case 0x90: autoinit = 1; start_block(block); break;
    case 0x40: rate = 1000000 / (256 - args[0]); if (getenv("DL_TRACE_SB")) fprintf(stderr, "sb: rate %d Hz\n", rate); break;
    case 0x41: rate = (args[0] << 8) | args[1]; break;
    case 0x48: block = (args[0] | args[1] << 8) + 1; break;
    case 0xd0: playing = 0; break;                                /* halt */
    case 0xd4: if (autoinit) playing = 1; break;                  /* continue */
    case 0xda: autoinit = 0; break;                               /* end auto-init after this block */
    case 0xd1: speaker = 1; break;
    case 0xd3: speaker = 0; break;
    case 0xd8: outq[outq_n++ & 15] = speaker ? 0xff : 0; break;
    case 0xe1: outq[outq_n++ & 15] = 3; outq[outq_n++ & 15] = 2; break;   /* DSP 3.02: Sound Blaster Pro 2 */
    case 0xe0: outq[outq_n++ & 15] = (u8)~args[0]; break;
    case 0xe4: mixer[0xfe] = args[0]; break;
    case 0xe8: outq[outq_n++ & 15] = mixer[0xfe]; break;
    case 0xf2: irq_pending = 1; break;
    default: break;
    }
}

static int nargs_for(u8 c)
{
    switch (c) {
    case 0x10: case 0x40: case 0xe0: case 0xe4: case 0x38: return 1;
    case 0x14: case 0x41: case 0x48: case 0x16: case 0x17: case 0x74: case 0x75: case 0x76: case 0x77: return 2;
    default: return 0;
    }
}

/* the DSP answers at any of the card's base addresses (210h-260h); FM is at base+0..3, 8, 9 */
static int is_dsp(u16 port)
{
    if (port < 0x210 || port > 0x26f) return 0;
    int r = port & 0xf;
    return r == 4 || r == 5 || r == 6 || r == 0xa || r == 0xc || r == 0xe;
}

int sb_out(u16 port, u8 v)
{
    if (port < 0x10 || (port >= 0x81 && port <= 0x87)) { dma_out(port, v); return 1; }
    if (!is_dsp(port)) return 0;
    switch (port & 0xf) {
    case 0x4: mixer_idx = v; break;
    case 0x5: mixer[mixer_idx] = v; break;
    case 0x6: sb_base = port & 0xfff0; if (v & 1) { outq_n = 0; playing = 0; autoinit = 0; nargs = want = 0; } else outq[outq_n++ & 15] = 0xaa; break;
    case 0xc:
        if (want) { args[nargs++] = v; if (nargs == want) { want = 0; command(); } }
        else { cmd = v; nargs = 0; want = nargs_for(v); if (!want) command(); }
        break;
    }
    return 1;
}

int sb_in(u16 port, u8 *v)
{
    if (port < 0x10) { *v = dma_in(port); return 1; }
    if (!is_dsp(port)) return 0;
    switch (port & 0xf) {
    case 0x5: *v = mixer[mixer_idx]; break;
    case 0xa:
        if (outq_n > 0) { *v = outq[0]; memmove(outq, outq + 1, 15); outq_n--; } else *v = 0xff;
        break;
    case 0xc: *v = 0x00; break;                                     /* always ready for a write */
    case 0xe: *v = outq_n > 0 ? 0xff : 0x7f; irq_pending = 0; break; /* data available; reading acknowledges */
    default: *v = 0xff;
    }
    return 1;
}

/* from dl_poll: the end of a block raises the card's interrupt */
int sb_poll(void)
{
    if (playing && plat_ticks_us() >= end_us) {
        if (autoinit) start_block(block); else playing = 0;
        irq_pending = 1;
        return 8 + sb_irq;                                           /* the interrupt vector */
    }
    return 0;
}

void sb_init(void)
{
    char p[1200]; snprintf(p, sizeof p, "%s/CONFIG.DRK", game_dir);
    FILE *f = fopen(p, "rb");
    if (!f) { snprintf(p, sizeof p, "%s/config.drk", game_dir); f = fopen(p, "rb"); }
    if (f) {
        u8 c[8] = {0};                           /* words: card, port, IRQ, DMA */
        if (fread(c, 1, 8, f) == 8) {
            int port = c[2] | c[3] << 8, irq = c[4] | c[5] << 8, dma = c[6] | c[7] << 8;
            if (port >= 0x210 && port <= 0x260) sb_base = port;
            if (irq >= 2 && irq <= 15) sb_irq = irq;
            if (dma >= 0 && dma <= 3) sb_dma = dma;
        }
        fclose(f);
    }
    if (getenv("DL_TRACE_SB")) fprintf(stderr, "sb: %03x irq %d dma %d\n", sb_base, sb_irq, sb_dma);
}
