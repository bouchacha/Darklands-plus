/* sfx.c - battle sound effects. The battle driver (PSOUND.DB and the others) makes every
 * combat sound on the FM chip or MIDI synthesizer ("random notes rather than clashing
 * swords"). Here they are played as recorded-style sounds instead: built-in ones synthesized
 * at start-up (no recordings ship with the port), or the player's own files
 * sounds\NAME.wav / NAME.ogg (NAME2.wav, NAME_3.ogg ... for variations, one picked at random).
 * Also new: hits make a sound (in the original only misses, parries and knock-backs do).
 *
 * The game's calls (s014): battle_sound(n) 0e76:465e checks the sound options and calls
 * the driver's sound_play(n) (047c:078a). n: 10 death, 11 handgun shot, 12 potion/cloud,
 * 13 effect marker, 14 hidden enemies appear, 15 bow/crossbow shot, 16 knock-back, 17 an
 * attack that fails (miss or parry), 18 "cannot go there". melee_step(i, set, again)
 * 0e76:199c resolves a blow on the frame where the weapon lands (cbt_anim[i] DS:ca57: 1/2
 * hit, 6 miss, else parried; frame counter combatant +0x1b, sequence length DS:c96b). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#define MKDIR(p) mkdir(p, 0755)
#endif
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "sfx.h"
#include "datamods.h"
#include "musicpack.h"

#define FS 44100
extern char game_dir[1024];
extern char dlp_user_dir[];
#define USER_DIR (dlp_user_dir[0] ? dlp_user_dir : game_dir)     /* the player's sounds folder lives here */
int sfx_mode = 1;               /* 0 the game's own sounds only, 1 a sound pack where it has a sound */
int sfx_synth = 0;              /* built-in synthesized sounds for what the pack lacks (experimental) */
int sfx_volume = 100;
int sfx_added_hits = 1;         /* hits make a sound too */
static int logon;

enum { S_MISS, S_PARRY, S_HIT, S_KNOCK, S_DEATH, S_BOW, S_GUN, S_BURST, S_MAGIC, S_AMBUSH, S_CANTGO, NS };
const char *SFX_NAMES[NS] = { "miss", "parry", "hit", "knockback", "death", "bow", "gun", "burst", "magic", "ambush", "cantgo" };
const char *SFX_DESC[NS] = { "an attack that misses", "a parried or blocked blow", "a blow that lands",
    "a heavy blow that knocks back", "a death", "a bow or crossbow shot", "a handgun shot", "a potion bursting",
    "a magical effect", "hidden enemies appear", "\"cannot go there\"" };
int SFX_COUNT = NS;

#define MAXVAR 8
typedef struct { short *s; int n; } Smp;
typedef struct { Smp v[MAXVAR]; int nv, user, last; } Bank;
static Bank bank[NS];

typedef struct { const short *s; int n; double pos, step; float gain; } Voice;
#define NVOICE 16
static Voice voice[NVOICE];
typedef struct { const short *s; int n; float step, gain; } Req;
static Req q[64];
static volatile unsigned qw, qr;


/* ---------------------------------------------------------------- synthesis */
static unsigned rs = 0x2545f491u;
static float frand(void) { rs ^= rs << 13; rs ^= rs >> 17; rs ^= rs << 5; return (rs >> 8) * (1.0f / 16777216.0f); }
static float nrand(void) { return frand() * 2 - 1; }

typedef struct { float ic1, ic2; } Svf;            /* trapezoidal state-variable filter */
static void svf(Svf *f, float x, float fc, float q, float *lp, float *bp, float *hp)
{
    if (fc > FS * 0.45f) fc = FS * 0.45f;
    if (fc < 10) fc = 10;
    float g = tanf((float)M_PI * fc / FS), k = 1.0f / q;
    float a1 = 1 / (1 + g * (g + k)), a2 = g * a1, a3 = g * a2;
    float v3 = x - f->ic2, v1 = a1 * f->ic1 + a2 * v3, v2 = f->ic2 + a2 * f->ic1 + a3 * v3;
    f->ic1 = 2 * v1 - f->ic1; f->ic2 = 2 * v2 - f->ic2;
    if (lp) *lp = v2;
    if (bp) *bp = v1;
    if (hp) *hp = x - k * v1 - v2;
}

static float *buf_new(float sec, int *n) { *n = (int)(sec * FS); return calloc((size_t)*n, sizeof(float)); }

/* a decaying sine from t0 (1 ms attack) */
static void partial(float *b, int n, float t0, float f, float amp, float tau)
{
    int i0 = (int)(t0 * FS);
    float ph = frand() * 6.283f;
    for (int i = i0; i < n; i++) {
        float t = (float)(i - i0) / FS, e = amp * expf(-t / tau) * (t < 0.001f ? t / 0.001f : 1);
        if (e < 1e-5f && t > 0.01f) break;
        b[i] += e * sinf(ph + 6.2831853f * f * t);
    }
}

/* a little metallic ping (two inharmonic partials) */
static void ping(float *b, int n, float t0, float f, float amp, float tau)
{
    partial(b, n, t0, f, amp, tau);
    partial(b, n, t0, f * 2.32f, amp * 0.5f, tau * 0.6f);
}

/* filtered noise with an exponential envelope; type 0 low, 1 band, 2 high pass */
static void noise(float *b, int n, float t0, float amp, float tau, int type, float fc, float q)
{
    int i0 = (int)(t0 * FS);
    Svf f = { 0, 0 };
    for (int i = i0; i < n; i++) {
        float t = (float)(i - i0) / FS, e = amp * expf(-t / tau) * (t < 0.0007f ? t / 0.0007f : 1);
        if (e < 1e-5f && t > 0.01f) break;
        float lp, bp, hp; svf(&f, nrand(), fc, q, &lp, &bp, &hp);
        b[i] += e * (type == 0 ? lp : type == 1 ? bp : hp);
    }
}

/* a low thump: sine sweeping down from f1 to f0 */
static void thump(float *b, int n, float t0, float f0, float f1, float amp, float tau)
{
    int i0 = (int)(t0 * FS);
    float ph = 0;
    for (int i = i0; i < n; i++) {
        float t = (float)(i - i0) / FS, e = amp * expf(-t / tau) * (t < 0.002f ? t / 0.002f : 1);
        if (e < 1e-5f && t > 0.01f) break;
        ph += 6.2831853f * (f0 + (f1 - f0) * expf(-t / 0.025f)) / FS;
        b[i] += e * sinf(ph);
    }
}

/* a voice (groan): a buzzing source through three formants */
static void groan(float *b, int n, float t0, float dur, float p0, float p1, float amp, int vowel)
{
    static const float F[2][3] = { { 700, 1150, 2600 }, { 520, 950, 2450 } };
    int i0 = (int)(t0 * FS), i1 = i0 + (int)(dur * FS);
    Svf f1 = { 0, 0 }, f2 = { 0, 0 }, f3 = { 0, 0 }, tilt = { 0, 0 };
    float ph = 0, jit = 0;
    for (int i = i0; i < i1 && i < n; i++) {
        float t = (float)(i - i0) / FS, u = t / dur;
        float env = amp * (t < 0.04f ? t / 0.04f : 1) * (u > 0.6f ? (1 - u) / 0.4f : 1);
        if ((i & 255) == 0) jit = nrand() * 0.03f;
        float p = (p0 + (p1 - p0) * u) * (1 + jit);
        ph += p / FS; if (ph >= 1) ph -= 1;
        float src = (2 * ph - 1) + 0.25f * nrand();
        float lp; svf(&tilt, src, 1800, 0.7f, &lp, 0, 0);
        float a, c, d;
        svf(&f1, lp, F[vowel][0], 6, 0, &a, 0); svf(&f2, lp, F[vowel][1], 8, 0, &c, 0); svf(&f3, lp, F[vowel][2], 12, 0, &d, 0);
        b[i] += env * (a + 0.6f * c + 0.3f * d);
    }
}

/* a small room for loud sounds: four damped combs and an all-pass */
static void room(float *b, int n, float wet)
{
    static const float D[4] = { 0.0297f, 0.0371f, 0.0411f, 0.0437f };
    float *out = calloc((size_t)n, sizeof(float));
    for (int k = 0; k < 4; k++) {
        int d = (int)(D[k] * FS);
        float *line = calloc((size_t)d, sizeof(float)), lpz = 0;
        for (int i = 0, p = 0; i < n; i++, p = (p + 1) % d) {
            float y = line[p];
            lpz = y * 0.6f + lpz * 0.4f;
            line[p] = b[i] + lpz * 0.74f;
            out[i] += y * 0.25f;
        }
        free(line);
    }
    int d = (int)(0.005f * FS);
    float *ap = calloc((size_t)d, sizeof(float));
    for (int i = 0, p = 0; i < n; i++, p = (p + 1) % d) {
        float z = ap[p], x = out[i] + z * 0.5f;
        ap[p] = x; out[i] = z - x * 0.5f;
    }
    for (int i = 0; i < n; i++) b[i] += wet * out[i];
    free(ap); free(out);
}

static Smp finish(float *b, int n, float peak)
{
    float m = 1e-9f;
    for (int i = 0; i < n; i++) if (fabsf(b[i]) > m) m = fabsf(b[i]);
    int last = n;
    while (last > 1 && fabsf(b[last - 1]) < m * 0.0005f) last--;
    int fade = FS / 100;                             /* 10 ms fade at the end */
    for (int i = 0; i < fade && i < last; i++) b[last - 1 - i] *= (float)i / fade;
    Smp s = { malloc(sizeof(short) * (size_t)last), last };
    for (int i = 0; i < last; i++) s.s[i] = (short)lrintf(b[i] / m * peak * 32767);
    free(b);
    return s;
}

static Smp make(int kind)
{
    int n; float *b;
    switch (kind) {
    case S_MISS: {                                   /* a whoosh: noise through a band that sweeps up and down */
        b = buf_new(0.4f, &n);
        float tp = 0.09f + 0.03f * frand(), top = 1400 + 900 * frand();
        Svf f = { 0, 0 }, g = { 0, 0 };
        for (int i = 0; i < n; i++) {
            float t = (float)i / FS, x = t / tp, env = x * x * expf(2 * (1 - x));
            float bp, lp; svf(&f, nrand(), 300 + top * env, 1.6f, 0, &bp, 0); svf(&g, nrand(), 500, 0.7f, &lp, 0, 0);
            b[i] = env * (bp + 0.25f * lp);
        }
        return finish(b, n, 0.40f);
    }
    case S_PARRY: {                                  /* steel on steel */
        b = buf_new(1.0f, &n);
        static const float R[5] = { 1, 2.76f, 5.40f, 8.93f, 13.34f }, A[5] = { 1, 0.6f, 0.45f, 0.3f, 0.2f }, T[5] = { 0.30f, 0.21f, 0.14f, 0.09f, 0.06f };
        float f0 = 1100 + 600 * frand();
        for (int k = 0; k < 5; k++) {
            float f = f0 * R[k] * (1 + 0.02f * nrand()), tau = T[k] * (0.8f + 0.4f * frand());
            if (f > 17000) continue;
            partial(b, n, 0, f, A[k], tau);
            partial(b, n, 0, f + 1.5f + 2 * frand(), A[k] * 0.7f, tau);
        }
        noise(b, n, 0, 1.4f, 0.002f, 2, 2500, 0.7f);
        noise(b, n, 0, 0.5f, 0.03f, 1, 3500 + 1500 * frand(), 2);
        thump(b, n, 0, 160, 320, 0.35f, 0.012f);
        return finish(b, n, 0.42f);
    }
    case S_HIT: {                                    /* a blow on armour and body */
        b = buf_new(0.35f, &n);
        thump(b, n, 0, 55 + 20 * frand(), 160, 1.0f, 0.06f + 0.02f * frand());
        noise(b, n, 0, 1.0f, 0.03f, 0, 800 + 500 * frand(), 0.7f);
        noise(b, n, 0, 0.6f, 0.008f, 1, 2200, 1);
        if (frand() < 0.6f)                          /* mail or plates rattling */
            for (int k = 0, m = 3 + (int)(frand() * 4); k < m; k++)
                ping(b, n, 0.004f + 0.07f * frand(), 3200 + 3800 * frand(), 0.08f + 0.12f * frand(), 0.012f + 0.02f * frand());
        return finish(b, n, 0.50f);
    }
    case S_KNOCK: {                                  /* a heavy blow and a grunt */
        b = buf_new(0.6f, &n);
        thump(b, n, 0, 45 + 15 * frand(), 130, 1.2f, 0.11f);
        noise(b, n, 0, 1.1f, 0.05f, 0, 600, 0.7f);
        noise(b, n, 0, 0.5f, 0.01f, 1, 1800, 1);
        for (int k = 0; k < 5; k++) ping(b, n, 0.01f + 0.12f * frand(), 2800 + 3500 * frand(), 0.1f + 0.1f * frand(), 0.02f + 0.02f * frand());
        groan(b, n, 0.02f, 0.22f, 150 + 30 * frand(), 110, 0.35f, 1);
        return finish(b, n, 0.55f);
    }
    case S_DEATH: {                                  /* a cry, the fall, armour settling */
        b = buf_new(1.2f, &n);
        groan(b, n, 0, 0.5f + 0.15f * frand(), 175 + 35 * frand(), 85, 0.45f, (int)(frand() * 2));
        float tf = 0.38f + 0.1f * frand();
        thump(b, n, tf, 38, 85, 1.2f, 0.18f);
        noise(b, n, tf, 0.9f, 0.10f, 0, 380, 0.7f);
        thump(b, n, tf + 0.2f, 45, 70, 0.4f, 0.1f);
        for (int k = 0; k < 9; k++) ping(b, n, tf + 0.4f * frand() * frand(), 2500 + 3500 * frand(), 0.08f + 0.15f * frand(), 0.02f + 0.03f * frand());
        return finish(b, n, 0.55f);
    }
    case S_BOW: {                                    /* the string and the arrow */
        b = buf_new(0.45f, &n);
        float f0 = 120 + 40 * frand(), ph[3] = { 0, 0, 0 };
        for (int i = 0; i < n; i++) {
            float t = (float)i / FS, f = f0 * (1 + 0.6f * expf(-t / 0.006f)), e = expf(-t / 0.08f);
            for (int k = 0; k < 3; k++) { ph[k] += 6.2831853f * f * (k + 1) / FS; b[i] += e * sinf(ph[k]) / (k + 1); }
        }
        noise(b, n, 0, 0.8f, 0.0015f, 2, 3000, 0.7f);
        Svf f = { 0, 0 };
        for (int i = (int)(0.01f * FS); i < n; i++) {
            float t = (float)i / FS - 0.01f, e = 0.45f * (t < 0.006f ? t / 0.006f : 1) * expf(-t / 0.07f);
            float bp; svf(&f, nrand(), 4200 - 2500 * t / 0.3f, 2, 0, &bp, 0);
            b[i] += e * bp;
        }
        return finish(b, n, 0.42f);
    }
    case S_GUN: {                                    /* a matchlock: crack, boom and the room */
        b = buf_new(1.8f, &n);
        noise(b, n, 0, 1.6f, 0.004f, 2, 1200, 0.6f);
        Svf f = { 0, 0 };
        for (int i = 0; i < n; i++) {
            float t = (float)i / FS, e = expf(-t / 0.2f) * (t < 0.001f ? t / 0.001f : 1);
            float lp; svf(&f, nrand(), 150 + 2600 * expf(-t / 0.05f), 0.8f, &lp, 0, 0);
            b[i] += 1.2f * e * lp;
        }
        thump(b, n, 0, 42, 90, 0.9f, 0.25f);
        room(b, n, 0.45f);
        return finish(b, n, 0.70f);
    }
    case S_BURST: {                                  /* a flask shatters and the contents flare up */
        b = buf_new(1.1f, &n);
        noise(b, n, 0, 0.8f, 0.02f, 2, 3000, 0.7f);
        for (int k = 0; k < 28; k++) {
            float t = 0.25f * frand() * frand();
            ping(b, n, t, 2500 + 6500 * frand(), 0.05f + 0.2f * frand() * frand(), 0.006f + 0.03f * frand());
        }
        Svf f = { 0, 0 };
        for (int i = 0; i < n; i++) {
            float t = (float)i / FS, x = t / 0.18f, e = 0.8f * powf(x, 1.5f) * expf(1.5f * (1 - x));
            float lp; svf(&f, nrand(), 150 + 900 * fminf(t / 0.35f, 1), 0.9f, &lp, 0, 0);
            b[i] += e * lp;
        }
        return finish(b, n, 0.52f);
    }
    }
    Smp z = { 0, 0 };
    return z;
}

/* ---------------------------------------------------------------- the player's files */
static int load_wav(const char *path, Smp *out)
{
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    unsigned char *d = malloc((size_t)n);
    if (!d || fread(d, 1, (size_t)n, f) != (size_t)n) { fclose(f); free(d); return 0; }
    fclose(f);
    int ok = 0, fmt = 0, ch = 0, rate = 0, bits = 0;
    if (n > 12 && !memcmp(d, "RIFF", 4) && !memcmp(d + 8, "WAVE", 4)) {
        long p = 12;
        while (p + 8 <= n) {
            long len = d[p + 4] | d[p + 5] << 8 | d[p + 6] << 16 | (long)d[p + 7] << 24;
            const unsigned char *c = d + p + 8;
            if (p + 8 + len > n) len = n - p - 8;
            if (!memcmp(d + p, "fmt ", 4) && len >= 16) {
                fmt = c[0] | c[1] << 8; ch = c[2] | c[3] << 8; rate = c[4] | c[5] << 8 | c[6] << 16 | c[7] << 24; bits = c[14] | c[15] << 8;
                if (fmt == 0xfffe && len >= 26) fmt = c[24] | c[25] << 8;
            } else if (!memcmp(d + p, "data", 4) && ch > 0 && rate > 0 && (fmt == 1 || fmt == 3)) {
                int bps = bits / 8;
                long frames = bps ? len / (bps * ch) : 0;
                float *m = malloc(sizeof(float) * (size_t)(frames + 1));
                for (long i = 0; i < frames; i++) {
                    float s = 0;
                    for (int k = 0; k < ch; k++) {
                        const unsigned char *q = c + (i * ch + k) * bps;
                        float v = 0;
                        if (fmt == 3 && bits == 32) { float fv; memcpy(&fv, q, 4); v = fv; }
                        else if (bits == 8) v = (q[0] - 128) / 128.0f;
                        else if (bits == 16) v = (short)(q[0] | q[1] << 8) / 32768.0f;
                        else if (bits == 24) v = (float)((int)((unsigned)q[0] << 8 | (unsigned)q[1] << 16 | (unsigned)q[2] << 24) >> 8) / 8388608.0f;
                        else if (bits == 32) v = (float)(int)((unsigned)q[0] | (unsigned)q[1] << 8 | (unsigned)q[2] << 16 | (unsigned)q[3] << 24) / 2147483648.0f;
                        s += v;
                    }
                    m[i] = s / ch;
                }
                long on = (long)((double)frames * FS / rate);
                out->s = malloc(sizeof(short) * (size_t)(on + 1)); out->n = (int)on;
                for (long i = 0; i < on; i++) {
                    double x = (double)i * rate / FS; long a = (long)x; float fr = (float)(x - a);
                    float v = a + 1 < frames ? m[a] * (1 - fr) + m[a + 1] * fr : (a < frames ? m[a] : 0);
                    if (v > 1) v = 1;
                    if (v < -1) v = -1;
                    out->s[i] = (short)lrintf(v * 32767);
                }
                free(m);
                ok = on > 0;
                break;
            }
            p += 8 + len + (len & 1);
        }
    }
    free(d);
    return ok;
}

int stb_vorbis_decode_filename(const char *filename, int *channels, int *sample_rate, short **output);
static int load_ogg(const char *path, Smp *out)
{
    int ch, rate; short *pcm;
    int frames = stb_vorbis_decode_filename(path, &ch, &rate, &pcm);
    if (frames <= 0 || ch <= 0 || rate <= 0) return 0;
    long on = (long)((double)frames * FS / rate);
    out->s = malloc(sizeof(short) * (size_t)(on + 1)); out->n = (int)on;
    for (long i = 0; i < on; i++) {
        long a = (long)((double)i * rate / FS);
        int s = 0;
        if (a < frames) for (int k = 0; k < ch; k++) s += pcm[a * ch + k];
        out->s[i] = (short)(s / ch);
    }
    free(pcm);
    return on > 0;
}

/* sounds\NAME.wav, NAME2.wav, NAME_3.ogg ... */
static int name_of(const char *file, char *key, int keyn)
{
    const char *dot = strrchr(file, '.');
    if (!dot || (strcasecmp(dot, ".wav") && strcasecmp(dot, ".ogg"))) return 0;
    int n = (int)(dot - file);
    while (n > 0 && isdigit((unsigned char)file[n - 1])) n--;
    if (n > 0 && file[n - 1] == '_') n--;
    if (n <= 0 || n >= keyn) return 0;
    for (int i = 0; i < n; i++) key[i] = (char)tolower((unsigned char)file[i]);
    key[n] = 0;
    return 1;
}

/* one sounds folder; a sound already given by a folder that wins (a mod's) is left alone */
static void load_user_dir(const char *dir, int level, int *from)
{
    DIR *d = opendir(dir);
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d))) {
        char key[32];
        if (!name_of(e->d_name, key, sizeof key)) continue;
        for (int k = 0; k < NS; k++) {
            if (strcmp(key, SFX_NAMES[k])) continue;
            Bank *B = &bank[k];
            if (from[k] >= 0 && from[k] != level) break;
            from[k] = level;
            if (!B->user) { for (int v = 0; v < B->nv; v++) free(B->v[v].s); B->nv = 0; B->user = 1; }
            if (B->nv >= MAXVAR) break;
            char p[1400]; snprintf(p, sizeof p, "%s/%s", dir, e->d_name);
            const char *dot = strrchr(e->d_name, '.');
            Smp s = { 0, 0 };
            if (!strcasecmp(dot, ".wav") ? load_wav(p, &s) : load_ogg(p, &s)) B->v[B->nv++] = s;
            else fprintf(stderr, "sounds: cannot read %s (PCM .wav or .ogg)\n", p);
        }
    }
    closedir(d);
}
static void load_user(void)
{
    int from[NS]; for (int k = 0; k < NS; k++) from[k] = -1;
    static char md[DATAMODS_MAX][1200];
    int nm = datamods_dirs("sounds", md, DATAMODS_MAX);
    for (int i = 0; i < nm; i++) load_user_dir(md[i], i, from);
    char dir[1100]; snprintf(dir, sizeof dir, "%s/sounds", USER_DIR);
    load_user_dir(dir, nm, from);
    if (strcmp(USER_DIR, game_dir)) { snprintf(dir, sizeof dir, "%s/sounds", game_dir); load_user_dir(dir, nm + 1, from); }   /* sounds put into a game folder that cannot be written */
}

int sfx_user_count(void) { int n = 0; for (int k = 0; k < NS; k++) n += bank[k].user ? bank[k].nv : 0; return n; }

void sound_lock(int on);
void sfx_reload(void)
{
    sound_lock(1);
    for (int i = 0; i < NVOICE; i++) voice[i].s = 0;
    qr = qw;
    for (int k = 0; k < NS; k++) { for (int v = 0; v < bank[k].nv; v++) free(bank[k].v[v].s); memset(&bank[k], 0, sizeof bank[k]); }
    rs = 0x2545f491u;
    for (int k = 0; k < NS && sfx_synth; k++) {
        if (k == S_MAGIC || k == S_AMBUSH || k == S_CANTGO) continue;   /* left to the game unless the player has files */
        int nv = k == S_HIT || k == S_MISS || k == S_PARRY ? 6 : 4;
        for (int v = 0; v < nv; v++) bank[k].v[bank[k].nv++] = make(k);
    }
    load_user();
    sound_lock(0);
}

/* ---------------------------------------------------------------- mixing */
static int play(int k, float gain)
{
    Bank *B = &bank[k];
    if (!B->nv) return 0;
    int v = (int)(frand() * B->nv);
    if (B->nv > 1 && v == B->last) v = (v + 1) % B->nv;
    B->last = v;
    if (qw - qr >= 64) return 1;
    Req *r = &q[qw & 63];
    r->s = B->v[v].s; r->n = B->v[v].n;
    r->step = B->user ? 1.0f : 1.0f + 0.08f * nrand();
    r->gain = gain * sfx_volume / 100.0f;
    __sync_synchronize();
    qw++;
    if (logon) fprintf(stderr, "sfx: %s (variant %d)\n", SFX_NAMES[k], v);
    return 1;
}

/* called from the audio callback: add the effects to the stereo stream */
void sfx_mix(short *out, int frames)
{
    while (qr != qw) {
        Req *r = &q[qr & 63];
        int slot = 0; double best = -1;
        for (int i = 0; i < NVOICE; i++) {
            if (!voice[i].s) { slot = i; best = -2; break; }
            if (voice[i].pos > best) { best = voice[i].pos; slot = i; }
        }
        voice[slot].s = r->s; voice[slot].n = r->n; voice[slot].pos = 0; voice[slot].step = r->step; voice[slot].gain = r->gain;
        __sync_synchronize();
        qr++;
    }
    for (int i = 0; i < NVOICE; i++) {
        Voice *v = &voice[i];
        if (!v->s) continue;
        for (int f = 0; f < frames; f++) {
            int a = (int)v->pos;
            if (a + 1 >= v->n) { v->s = 0; break; }
            float fr = (float)(v->pos - a), x = (v->s[a] * (1 - fr) + v->s[a + 1] * fr) * v->gain;
            for (int c = 0; c < 2; c++) {
                int y = out[f * 2 + c] + (int)x;
                out[f * 2 + c] = (short)(y > 32767 ? 32767 : y < -32768 ? -32768 : y);
            }
            v->pos += v->step;
        }
    }
}

/* ---------------------------------------------------------------- hooks */
static int want = -1;            /* the sound battle_sound() is asking for */
static int impact = -1, impact_anim, impact_done;

static void battle_sound_hook(void)          /* battle_sound(n) */
{
    int old = want;
    want = (s16)rw(SS, (u16)(SP + 4));
    hook_call_original();
    want = old;
}

static void sound_play_hook(void)            /* sound_play(n): the driver */
{
    int n = (s16)rw(SS, (u16)(SP + 4));
    musicpack_cmd(n);                         /* the music (towns' driver) */
    if (sfx_mode && want == n && n >= 10 && n <= 18) {
        int k = -1; float g = 1;
        switch (n) {
        case 10: k = S_DEATH; break;
        case 11: k = S_GUN; break;
        case 12: k = S_BURST; break;
        case 13: k = S_MAGIC; break;
        case 14: k = S_AMBUSH; break;
        case 15: k = S_BOW; break;
        case 16: k = S_KNOCK; break;
        case 17: k = impact >= 0 && impact_anim == 6 ? S_MISS : S_PARRY; break;
        case 18: k = S_CANTGO; break;
        }
        if (k >= 0 && play(k, g)) { impact_done = 1; hook_return_far(0); return; }
    }
    if (n >= 10 && n <= 18) impact_done = 1;
    hook_call_original();
}

static void melee_hook(void)                 /* melee_step(i, set, *again), near */
{
    int i = (s16)rw(SS, (u16)(SP + 2));
    int land = 0, anim = 0;
    if (i >= 0 && i < 13) {
        int f1b = (s8)g_rb((u16)(GV_combatants + 0x80 * i + 0x1b)), len = (s8)g_rb((u16)(GV_cbt_anim_len + i));
        anim = g_rb((u16)(GV_cbt_anim + i));
        land = f1b + 1 == len - 1;
    }
    int oi = impact, oa = impact_anim, od = impact_done;
    impact = land ? i : -1; impact_anim = anim; impact_done = 0;
    int tg = i >= 0 && i < 13 ? (s8)g_rb((u16)(GV_combatants + 0x80 * i + 0x4a)) : -1;
    int e0 = tg >= 0 && tg < 13 ? (s8)g_rb((u16)(GV_combatants + 0x80 * tg + 0x5d)) : 0, s0 = tg >= 0 && tg < 13 ? (s8)g_rb((u16)(GV_combatants + 0x80 * tg + 0x5e)) : 0;
    hook_call_original();
    if (land && tg >= 0 && tg < 13) { void battlelog_blow(int, int, int, int, int, int, int); battlelog_blow(i, tg, anim, e0, (s8)g_rb((u16)(GV_combatants + 0x80 * tg + 0x5d)), s0, (s8)g_rb((u16)(GV_combatants + 0x80 * tg + 0x5e))); }
    if (land && getenv("DL_BLOW_LOG") && tg >= 0 && tg < 13) { u32 plat_ticks_ms(void); fprintf(stderr, "blow %u: %d -> %d anim %d: end %d -> %d, str %d -> %d\n", plat_ticks_ms(), i, tg, anim, e0, (s8)g_rb((u16)(GV_combatants + 0x80 * tg + 0x5d)), s0, (s8)g_rb((u16)(GV_combatants + 0x80 * tg + 0x5e))); }
    if (land && (anim == 1 || anim == 2) && !impact_done && sfx_mode && sfx_added_hits && g_rb(GV_opt_sound) == 1)
        play(S_HIT, anim == 2 ? 1.0f : 0.7f);
    impact = oi; impact_anim = oa; impact_done = od;
}

void sfx_set(int mode) { sfx_mode = mode; config_set_int("sound.effects", mode); }
void sfx_set_volume(int v) { sfx_volume = v; config_set_int("sound.effects_volume", v); }
void sfx_set_hits(int on) { sfx_added_hits = on; config_set_int("sound.hit_sounds", on); }
void sfx_set_synth(int on) { sfx_synth = on; config_set_int("sound.synth_effects", on); sfx_reload(); }

/* write the built-in sounds as .wav (for listening, or as a start for one's own) */
int sfx_export(char *msg, int n)
{
    char dir[1200]; snprintf(dir, sizeof dir, "%s/sounds", USER_DIR); MKDIR(dir);
    snprintf(dir, sizeof dir, "%s/sounds/built-in", USER_DIR); MKDIR(dir);
    unsigned save = rs; rs = 0x2545f491u;
    int done = 0;
    for (int k = 0; k < NS; k++) {
        if (k == S_MAGIC || k == S_AMBUSH || k == S_CANTGO) continue;
        int nv = k == S_HIT || k == S_MISS || k == S_PARRY ? 6 : 4;
        for (int v = 0; v < nv; v++) {
            Smp s = make(k);
            char p[1400]; snprintf(p, sizeof p, "%s/%s%d.wav", dir, SFX_NAMES[k], v + 1);
            FILE *f = fopen(p, "wb");
            if (f) {
                unsigned bytes = (unsigned)s.n * 2;
                unsigned h[11] = { 0x46464952, 36 + bytes, 0x45564157, 0x20746d66, 16, 0x00010001, FS, FS * 2, 0x00100002, 0x61746164, bytes };
                fwrite(h, 4, 11, f); fwrite(s.s, 2, (size_t)s.n, f); fclose(f); done++;
            }
            free(s.s);
        }
    }
    rs = save;
    snprintf(msg, (size_t)n, "%d sounds written to %s", done, dir);
    return done;
}

void sfx_init(void)
{
    logon = getenv("DL_SFX_LOG") != 0;
    sfx_mode = config_get_int("sound.effects", 1);
    sfx_volume = config_get_int("sound.effects_volume", 100);
    sfx_added_hits = config_get_int("sound.hit_sounds", 1);
    sfx_synth = config_get_int("sound.synth_effects", 0);
    sfx_reload();
    hook_install(14, 0x0e76, 0x465e, battle_sound_hook);
    hook_install(GF_sound_play, sound_play_hook);
    if (!hook_install(14, 0x0e76, 0x199c, melee_hook)) fprintf(stderr, "sfx: no hook point for melee_step\n");
    if (getenv("DL_SFX_EXPORT")) { char m[300]; sfx_export(m, sizeof m); fprintf(stderr, "%s\n", m); }
}
