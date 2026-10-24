/* musicpack.c - music packs: recordings played in place of the game's synthesizer music.
 *
 * The game asks its sound driver (xSOUND.DLC: towns, the map, the opening and the ending) for
 * a piece by number with sound_play(n) (root 047c:078a; music_play(n), s043 1462:09ae, asks
 * only while the music is on). Every driver (Roland, Sound Blaster, AdLib, PC speaker) knows
 * the same numbers:
 *   0 reset, 1 stop at once, 2 pause (mute and hold), 3 go on, 5 which piece is playing, 6 fade out,
 *   8-47 pieces (a new one replaces the last one; 33 and 34 are short sounds over it; 47 is
 *   the opening, 45 the ending), 48-59 settings sent to the synthesizer (no music).
 * (The driver's table of handlers is at 0f4c in RSOUND.DLC; a piece loads its instruments,
 * stops the tracks and starts its own, up to nine, and returns at once if it is playing.)
 * A music pack is a folder "music" (in a mod folder, or in the Darklands+ folder of the
 * player's own files) with files named by those numbers: 10.ogg, "10 tavern.ogg", 09.wav ...
 * (Ogg Vorbis or PCM .wav, any sample rate, mono or stereo). A piece the pack has is played
 * from its file and the synthesizer is silenced meanwhile; a piece the pack lacks plays from
 * the synthesizer as before. A piece that loops in the game loops in the pack (the whole file
 * again), one that plays once plays once. The driver goes on playing unheard, so the game
 * itself sees no difference. Battles have no music (their driver only makes the sounds). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <dirent.h>
#include "../runtime/cpu.h"
#include "../runtime/config.h"
#include "datamods.h"
#include "musicpack.h"
#define STB_VORBIS_HEADER_ONLY
#include "../third_party/stb/stb_vorbis.c"

#define FS 44100
#define NSONG 60
extern char game_dir[1024];
extern char dlp_user_dir[];
#define USER_DIR (dlp_user_dir[0] ? dlp_user_dir : game_dir)
void sound_lock(int on);
extern int sound_music_volume;

int musicpack_on = 1;                            /* sound.music_pack */
#define MAXV 4                                   /* versions of one piece, played in turn (27: the driver has four) */
static char file[NSONG][MAXV][1300];             /* each piece's recordings ("" none) */
static int nvar[NSONG], turn[NSONG];
static char from[NSONG][96];                     /* and the folder (mod) it came from */
static int nfiles;
static int logon, in_battle, last_song = -1, last_from_pack;
static volatile int owns;                        /* the piece the game is playing is the pack's: the synthesizer is silenced */

static int is_overlay(int n) { return n == 33 || n == 34; }
#define LAST_PIECE 47

/* the pieces that play once and end (the others loop): found by playing each piece for five
 * minutes on the Roland driver with DL_MUSIC_PROBE=1 (its tracks logged; ucall:-1,047c,078a,N).
 * Their lengths there: 8 59 s, 9 115, 10 56, 13 31, 16 59, 17 66, 18 44, 19 116, 22 102,
 * 27 5 (one of four), 28 72, 29 58, 30 6, 33 5, 34 0.2, 36 15, 37 26, 41 76, 42 17, 43 47,
 * 44 56, 45 76. */
static const unsigned char ONCE[NSONG] = {
    [8] = 1, [9] = 1, [10] = 1, [13] = 1, [16] = 1, [17] = 1, [18] = 1, [19] = 1, [22] = 1, [27] = 1,
    [28] = 1, [29] = 1, [30] = 1, [33] = 1, [34] = 1, [36] = 1, [37] = 1, [41] = 1, [42] = 1,
    [43] = 1, [44] = 1, [45] = 1,
};
int musicpack_loops(int n) { return n >= 8 && n <= 47 && !ONCE[n] && n != 33 && n != 34; }

/* ---------------------------------------------------------------- the files */
static int number_of(const char *name)
{
    const char *dot = strrchr(name, '.');
    if (!dot || (strcasecmp(dot, ".ogg") && strcasecmp(dot, ".wav"))) return -1;
    if (!isdigit((unsigned char)name[0])) return -1;
    int n = 0, k = 0;
    while (isdigit((unsigned char)name[k]) && k < 3) n = n * 10 + (name[k++] - '0');
    if (isdigit((unsigned char)name[k])) return -1;
    return n >= 8 && n <= LAST_PIECE ? n : -1;
}
static int by_name(const void *a, const void *b) { return strcasecmp(*(char *const *)a, *(char *const *)b); }
static void scan_dir(const char *dir, const char *label)
{
    DIR *d = opendir(dir);
    if (!d) return;
    char *names[512]; int nn = 0;
    struct dirent *e;
    while ((e = readdir(d)) && nn < 512)
        if (number_of(e->d_name) >= 0) names[nn++] = strdup(e->d_name);
    closedir(d);
    qsort(names, (size_t)nn, sizeof names[0], by_name);
    int had[NSONG]; for (int k = 0; k < NSONG; k++) had[k] = nvar[k] > 0;   /* a folder that wins already has it */
    for (int i = 0; i < nn; i++) {
        int n = number_of(names[i]);
        if (!had[n] && nvar[n] < MAXV) {
            if (!nvar[n]) nfiles++;
            snprintf(file[n][nvar[n]++], sizeof file[0][0], "%s/%s", dir, names[i]);
            snprintf(from[n], sizeof from[n], "%s", label);
        }
        free(names[i]);
    }
}

static void stop_all(float fade, int give_back);
void musicpack_reload(void)
{
    stop_all(0.05f, 1);
    memset(file, 0, sizeof file); memset(nvar, 0, sizeof nvar); nfiles = 0;
    if (config_get_int("original", 0)) return;
    static char md[DATAMODS_MAX][1200];
    int nm = datamods_dirs("music", md, DATAMODS_MAX);
    for (int i = 0; i < nm; i++) {
        const char *p = md[i] + strlen(md[i]);       /* the mod's name: the folder above "music" */
        while (p > md[i] && p[-1] != '/' && p[-1] != '\\') p--;
        const char *q = p > md[i] ? p - 1 : p, *s = q;
        while (s > md[i] && s[-1] != '/' && s[-1] != '\\') s--;
        char label[96]; snprintf(label, sizeof label, "%.*s", (int)(q - s), s);
        scan_dir(md[i], label);
    }
    char dir[1200]; snprintf(dir, sizeof dir, "%s/music", USER_DIR); scan_dir(dir, "music");
    if (strcmp(USER_DIR, game_dir)) { snprintf(dir, sizeof dir, "%s/music", game_dir); scan_dir(dir, "music"); }
    if (nfiles) fprintf(stderr, "music pack: %d piece%s\n", nfiles, nfiles == 1 ? "" : "s");
}
/* what the game plays them for, where known (the card that asks for each) */
const char *musicpack_desc(int n)
{
    switch (n) {
    case 8: return "a church";
    case 9: case 10: return "the inn (one of the two at random)";
    case 11: return "the market";
    case 13: return "praying to a saint";
    case 12: return "the guilds and craftsmen";
    case 14: return "the city square";
    case 15: return "the university";
    case 16: return "the shell game";
    case 17: return "the fortress";
    case 18: return "the party's end";
    case 19: return "the map (one of two at random)";
    case 20: return "the map (one of two at random); making the party";
    case 22: return "the dungeon";
    case 23: return "a grove in town by day";
    case 24: return "the dragon's lair";
    case 27: return "before a city by day (four tunes in turn)";
    case 28: return "before a city by night";
    case 29: return "a grove in town by night";
    case 31: case 32: return "an ambush, or something hidden ahead";
    case 35: return "the wild hunt";
    case 36: return "the woodwife";
    case 37: return "the schrat";
    case 38: case 39: return "a dream in camp";
    case 40: return "the manor by the misty lake";
    case 41: return "a village";
    case 42: return "the Templars";
    case 45: return "the ending";
    case 47: return "the opening";
    }
    if (n == 33 || n == 34) return "a short sound played over the current piece";
    return "";
}

int musicpack_count(void) { return nfiles; }
const char *musicpack_file(int n) { return n >= 0 && n < NSONG ? file[n][0] : ""; }
int musicpack_versions(int n) { return n >= 0 && n < NSONG ? nvar[n] : 0; }
const char *musicpack_from(int n) { return n >= 0 && n < NSONG ? from[n] : ""; }

/* ---------------------------------------------------------------- streams */
typedef struct {
    volatile int on;
    int song, loop, paused;
    stb_vorbis *v; FILE *f;                      /* an Ogg file, or a .wav's data */
    long wav_data, wav_len, wav_left; int bits, block;
    int ch, rate;
    short src[2 * 2048]; int sn, si;             /* decoded, stereo */
    short a[2], b[2]; double frac, step;         /* resampling: between a and b */
    float gain, target, rate_g;                  /* fading: gain moves to target by rate_g a frame */
} Stream;
static Stream st[3];                             /* 0 the piece, 1 a short one over it, 2 the last piece fading out */

static void s_close(Stream *s)
{
    if (s->v) stb_vorbis_close(s->v);
    if (s->f) fclose(s->f);
    s->v = 0; s->f = 0; s->on = 0;
}

static int wav_open(Stream *s, const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    unsigned char h[12];
    if (fread(h, 1, 12, f) != 12 || memcmp(h, "RIFF", 4) || memcmp(h + 8, "WAVE", 4)) { fclose(f); return 0; }
    int fmt = 0;
    for (;;) {
        unsigned char c[8];
        if (fread(c, 1, 8, f) != 8) break;
        long len = (long)(c[4] | c[5] << 8 | c[6] << 16 | (unsigned)c[7] << 24);
        if (!memcmp(c, "fmt ", 4) && len >= 16) {
            unsigned char x[16];
            if (fread(x, 1, 16, f) != 16) break;
            int tag = x[0] | x[1] << 8;
            s->ch = x[2] | x[3] << 8; s->rate = (int)(x[4] | x[5] << 8 | x[6] << 16 | (unsigned)x[7] << 24);
            s->bits = x[14] | x[15] << 8; s->block = x[12] | x[13] << 8;
            fmt = (tag == 1 || tag == 0xfffe) && s->ch >= 1 && s->ch <= 8 && s->rate > 0 && (s->bits == 8 || s->bits == 16 || s->bits == 24) && s->block > 0;
            fseek(f, len - 16 + (len & 1), SEEK_CUR);
        }
        else if (!memcmp(c, "data", 4)) {
            if (!fmt) break;
            s->f = f; s->wav_data = ftell(f); s->wav_len = len - len % s->block; s->wav_left = s->wav_len;
            return 1;
        }
        else fseek(f, len + (len & 1), SEEK_CUR);
    }
    fclose(f);
    return 0;
}

static int s_open(Stream *s, int song)
{
    memset(s, 0, sizeof *s);
    const char *p = file[song][turn[song]++ % (nvar[song] ? nvar[song] : 1)];
    const char *dot = strrchr(p, '.');
    if (dot && !strcasecmp(dot, ".ogg")) {
        int err = 0;
        s->v = stb_vorbis_open_filename(p, &err, NULL);
        if (!s->v) { fprintf(stderr, "music pack: cannot read %s\n", p); return 0; }
        stb_vorbis_info in = stb_vorbis_get_info(s->v);
        s->ch = in.channels; s->rate = (int)in.sample_rate;
    }
    else if (!wav_open(s, p)) { fprintf(stderr, "music pack: cannot read %s (Ogg Vorbis, or PCM .wav)\n", p); return 0; }
    s->song = song;
    s->step = (double)s->rate / FS;
    return 1;
}

/* from the audio thread: the next block of the file, stereo; 0 at its end */
static int s_fill(Stream *s)
{
    s->si = 0; s->sn = 0;
    if (s->v) s->sn = stb_vorbis_get_samples_short_interleaved(s->v, 2, s->src, 2 * 2048);
    else if (s->f && s->wav_left > 0) {
        unsigned char raw[2048 * 8 * 3];
        long want = (long)(sizeof raw / (size_t)s->block) * s->block;
        if (want > 2048L * s->block) want = 2048L * s->block;
        if (want > s->wav_left) want = s->wav_left;
        long got = (long)fread(raw, 1, (size_t)want, s->f);
        got -= got % s->block;
        s->wav_left -= got;
        int frames = (int)(got / s->block), bps = s->bits / 8;
        for (int i = 0; i < frames; i++) {
            int v[2];
            for (int c = 0; c < 2; c++) {
                const unsigned char *q = raw + (size_t)i * s->block + (size_t)(c < s->ch ? c : 0) * bps;
                v[c] = bps == 1 ? (q[0] - 128) << 8 : bps == 2 ? (short)(q[0] | q[1] << 8) : (short)(q[1] | q[2] << 8);
            }
            s->src[2 * i] = (short)v[0]; s->src[2 * i + 1] = (short)v[1];
        }
        s->sn = frames;
        if (got <= 0) s->wav_left = 0;
    }
    return s->sn > 0;
}
static void s_rewind(Stream *s)
{
    if (s->v) stb_vorbis_seek_start(s->v);
    else if (s->f) { fseek(s->f, s->wav_data, SEEK_SET); s->wav_left = s->wav_len; }
}
/* the next source frame into b; 0 when the piece is over */
static int s_next(Stream *s)
{
    s->a[0] = s->b[0]; s->a[1] = s->b[1];
    if (s->si >= s->sn && !s_fill(s)) {
        if (!s->loop) return 0;
        s_rewind(s);
        if (!s_fill(s)) return 0;
    }
    s->b[0] = s->src[2 * s->si]; s->b[1] = s->src[2 * s->si + 1]; s->si++;
    return 1;
}

static void fade_to(Stream *s, float target, float sec)
{
    s->target = target;
    s->rate_g = sec <= 0 ? 1.0f : 1.0f / (sec * FS);
}

/* DL_MUSIC_PROBE: which of the Roland driver's nine tracks play, whenever that changes (for
 * telling the pieces that loop from those that end) */
static void probe(void)
{
    u16 driver_seg_by_sig(const char *sig);
    u16 seg = driver_seg_by_sig("RLND DrkLand03-16-93");
    if (!seg) return;
    u32 ds = (u32)(seg + 0x224) << 4;
    int mask = 0;
    for (int k = 0; k < 9; k++) if (M[ds + 0xcdd0 + 0x22 * k]) mask |= 1 << k;
    static int last = -1;
    if (mask != last) { u32 plat_ticks_ms(void); fprintf(stderr, "probe %u: %03x\n", plat_ticks_ms(), mask); last = mask; }
}

/* from the audio callback: the pieces added to the output */
void musicpack_mix(short *out, int frames)
{
    { static int pr = -1; if (pr < 0) pr = getenv("DL_MUSIC_PROBE") != 0; if (pr) probe(); }
    for (int k = 0; k < 3; k++) {
        Stream *s = &st[k];
        if (!s->on || s->paused) continue;
        for (int i = 0; i < frames; i++) {
            while (s->frac >= 1.0) { s->frac -= 1.0; if (!s_next(s)) { s_close(s); break; } }
            if (!s->on) break;
            if (s->gain != s->target) {
                if (s->gain < s->target) { s->gain += s->rate_g; if (s->gain > s->target) s->gain = s->target; }
                else { s->gain -= s->rate_g; if (s->gain <= s->target) { s->gain = s->target; if (s->target <= 0) { s_close(s); break; } } }
            }
            float g = s->gain * (float)sound_music_volume / 100.0f, fr = (float)s->frac;
            for (int c = 0; c < 2; c++) {
                int y = out[2 * i + c] + (int)((s->a[c] + (s->b[c] - s->a[c]) * fr) * g);
                out[2 * i + c] = (short)(y > 32767 ? 32767 : y < -32768 ? -32768 : y);
            }
            s->frac += s->step;
        }
    }
}

/* The synthesizer is silenced from the start of a piece of the pack until the game starts
 * a piece the pack does not have (or a battle begins): also after a recording that plays once
 * has ended, as the driver's own may go on longer, and after a stop, while the driver's piece
 * fades out unheard. */
int musicpack_mutes(void) { return owns; }

/* the pieces fade out; with give_back the synthesizer is heard again at once */
static void stop_all(float fade, int give_back)
{
    sound_lock(1);
    if (st[0].on) {
        if (st[2].on) s_close(&st[2]);
        st[2] = st[0]; st[0].on = 0; st[0].v = 0; st[0].f = 0;
    }
    for (int k = 1; k < 3; k++) if (st[k].on) fade_to(&st[k], 0, fade);
    if (give_back) owns = 0;
    sound_lock(0);
}

static void start(int song)
{
    int over = is_overlay(song);
    Stream ns;
    if (!s_open(&ns, song)) return;
    ns.loop = !over && !ONCE[song];
    ns.gain = 0; fade_to(&ns, 1, over ? 0.005f : 0.05f);
    ns.frac = 1.0;                               /* the first frame read at once */
    sound_lock(1);
    if (!over) {
        if (st[2].on) s_close(&st[2]);
        if (st[0].on) { st[2] = st[0]; fade_to(&st[2], 0, 0.4f); st[0].on = 0; st[0].v = 0; st[0].f = 0; }
        if (st[1].on) fade_to(&st[1], 0, 0.4f);
        st[0] = ns; st[0].on = 1;
        owns = 1;
    } else {
        if (st[1].on) s_close(&st[1]);
        st[1] = ns; st[1].on = 1;
    }
    sound_lock(0);
    if (logon) fprintf(stderr, "music pack: %d (%d of %d)%s\n", song, (turn[song] - 1) % nvar[song] + 1, nvar[song], ns.loop ? " (loops)" : "");
}

/* sound_play(n) (sfx.c's hook, before the driver) */
void musicpack_cmd(int n)
{
    if (logon) { u32 plat_ticks_ms(void); fprintf(stderr, "music %u: %d%s\n", plat_ticks_ms(), n, in_battle ? " (battle driver)" : ""); }
    if (in_battle) return;
    { extern int musicpack_held; if (musicpack_held) return; }   /* (a sound device heard on its own: mods/soundpreview.c) */
    if (n >= 8 && n <= LAST_PIECE && !is_overlay(n)) { last_song = n; last_from_pack = musicpack_on && nvar[n] > 0; }
    else if (n == 0 || n == 1 || n == 6) last_song = -1;
    if (!musicpack_on || !nfiles) { if (owns || st[0].on || st[1].on || st[2].on) stop_all(0.3f, 1); return; }
    switch (n) {
    case 0: case 1: stop_all(0.1f, 0); return;   /* the driver's piece stops too (gradually with 6): heard no more */
    case 6: stop_all(1.5f, 0); return;
    case 2: case 3:
        sound_lock(1); for (int k = 0; k < 3; k++) st[k].paused = n == 2; sound_lock(0);
        return;
    }
    if (n < 8 || n > LAST_PIECE) return;
    if (!nvar[n]) {
        if (!is_overlay(n)) stop_all(0.3f, 1);   /* the synthesizer plays it */
        else if (!st[0].on) owns = 0;            /* a short sound after the pack's piece: heard from the synthesizer */
        return;
    }
    if (!is_overlay(n) && st[0].on && st[0].song == n && st[0].target > 0) return;   /* playing already: the driver does not start it again either */
    start(n);
}

/* a sound driver is loaded (drivers.c): the battle driver plays no music */
void musicpack_driver(const char *filename)
{
    size_t l = strlen(filename);
    int c = l ? toupper((unsigned char)filename[l - 1]) : 0;   /* xSOUND.DLC / .DC towns, .DLB / .DB battles */
    if (c != 'B' && c != 'C') return;
    int b = c == 'B';
    in_battle = b;
    if (b) { stop_all(0.3f, 1); last_song = -1; }
}

/* the last piece the game started, and whether the pack played it */
int musicpack_now(int *from_pack) { if (from_pack) *from_pack = last_from_pack; return last_song; }

void musicpack_hold(int on) { if (on) stop_all(0.2f, 1); }
/* the settings' preview of the pack: piece n from its recording for a while (n < 0: stop) */
void musicpack_preview(int n)
{
    if (n < 0 || n >= NSONG || !nvar[n]) { stop_all(0.5f, 1); return; }
    start(n);
}
int musicpack_first_piece(void)
{
    static const int PREF[] = { 11, 9, 10, 27, 19, 20, 12, 14 };
    for (int i = 0; i < 8; i++) if (nvar[PREF[i]]) return PREF[i];
    for (int k = 8; k <= LAST_PIECE; k++) if (nvar[k]) return k;
    return -1;
}
void musicpack_set(int on) { musicpack_on = on; config_set_int("sound.music_pack", on); if (!on) stop_all(0.5f, 1); }

void musicpack_init(void)
{
    logon = getenv("DL_MUSIC_LOG") != 0;
    musicpack_on = config_get_int("sound.music_pack", 1);
    musicpack_reload();
}
