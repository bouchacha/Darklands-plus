/* mt32.c - a Roland MT-32 / CM-32L / LAPC-I inside Darklands+ (Munt's libmt32emu, LGPL 2.1+,
 * third_party/mt32emu).
 *
 * Darklands' Roland driver (RSOUND.DLC for towns and the map, RSOUND.DLB for battles) is written
 * for the MT-32 and the LAPC-I ("Roland MT-32/LAPC-1", README.TXT): before playing it loads its
 * own patches and timbres into the synthesizer by SysEx (the timbres are called "Potion",
 * "Monster", "CatAlien" ...) and plays both the music and the battle sounds with them. A General
 * MIDI synthesizer (Windows' own, or a Sound Canvas) ignores all that and plays the numbers as
 * its own instruments, which is why the GOG release sounds like "random notes". Here the MPU-401
 * stream goes into an emulated MT-32 instead, mixed into the game's sound output, so nothing
 * needs to be installed or routed.
 *
 * The emulator needs the synthesizer's two ROM images (control and PCM), which are Roland's and
 * do not come with Darklands+. They are looked for, by their contents (any file names), in
 * mt32_dirs(): the folder set in F12 > Settings, "roms" folders of Darklands+ and of the game,
 * Munt's own ROM folder (its registry setting, or %USERPROFILE%\roms) and DOSBox Staging's
 * mt32-roms folder. An MT-32 pair is preferred by default: on a CM-32L / LAPC-I the battle sounds'
 * notes reach its extra sound-effect samples and come out as chimes instead of blows and grunts;
 * F12 > Settings can prefer the CM-32L. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>
#include "../third_party/mt32emu/mt32emu.h"
#include "config.h"
#ifdef _WIN32
#include <windows.h>
#endif

extern char game_dir[1024];
extern char dlp_user_dir[], dlp_data_dir[], dlp_prog_dir[];
void sound_lock(int on);

static mt32emu_context ctx;
static int state;                       /* 0 not tried, 1 open, -1 cannot */
static char found_desc[300], found_dir[1100], why[300];
static int scanned;                     /* the ROM search done (its result: chosen[]) */

#define MAXF 64
typedef struct { char path[1100]; const char *ctrl, *pcm; } RomFile;
static RomFile files[MAXF]; static int nfiles;
static char machine[40];                /* the machine whose ROMs were found */
static char playing[40];                /* the machine the emulator was started as */

/* ---------------------------------------------------------------- where to look */
static int ndirs; static char dirs[24][1100];
static void add_dir(const char *d)
{
    if (!d || !d[0] || ndirs >= 24) return;
    for (int i = 0; i < ndirs; i++) if (!strcmp(dirs[i], d)) return;
    snprintf(dirs[ndirs++], sizeof dirs[0], "%s", d);
}
static void add_sub(const char *base, const char *sub)
{
    if (!base || !base[0]) return;
    char p[1100]; snprintf(p, sizeof p, "%s/%s", base, sub); add_dir(p);
}
#ifdef _WIN32
static int reg_str(HKEY root, const char *key, const char *name, char *out, DWORD n)
{
    HKEY h; if (RegOpenKeyExA(root, key, 0, KEY_READ, &h) != ERROR_SUCCESS) return 0;
    DWORD type = 0, len = n - 1; int ok = RegQueryValueExA(h, name, 0, &type, (BYTE *)out, &len) == ERROR_SUCCESS && type == REG_SZ;
    RegCloseKey(h);
    if (ok) out[len < n ? len : n - 1] = 0;
    return ok && out[0];
}
#endif
static void mt32_dirs(void)
{
    ndirs = 0;
    add_dir(config_get("sound.mt32_roms", ""));
    add_sub(dlp_user_dir, "roms"); add_sub(game_dir, "roms"); add_sub(dlp_prog_dir, "roms"); add_sub(dlp_data_dir, "roms");
    add_dir(game_dir); add_dir(dlp_prog_dir);
#ifdef _WIN32
    {   /* Munt (mt32emu-qt and its Windows driver): Profiles\<default profile>\romDir */
        char prof[200] = "default", key[400], d[1100];
        reg_str(HKEY_CURRENT_USER, "Software\\muntemu.org\\Munt mt32emu-qt\\Master", "defaultSynthProfile", prof, sizeof prof);
        snprintf(key, sizeof key, "Software\\muntemu.org\\Munt mt32emu-qt\\Profiles\\%s", prof);
        if (reg_str(HKEY_CURRENT_USER, key, "romDir", d, sizeof d)) add_dir(d);
    }
    const char *up = getenv("USERPROFILE"), *la = getenv("LOCALAPPDATA");
    add_sub(up, "roms");                                   /* Munt's default */
    add_sub(la, "DOSBox\\mt32-roms");                      /* DOSBox Staging */
    add_sub(up, "mt32-roms");
#else
    const char *home = getenv("HOME");
    add_sub(home, "roms"); add_sub(home, ".config/dosbox/mt32-roms"); add_sub(home, ".local/share/dosbox/mt32-roms");
#endif
}

/* ---------------------------------------------------------------- finding the ROMs */
static int rom_size(long n) { return n == 32768 || n == 65536 || n == 131072 || n == 262144 || n == 524288 || n == 1048576; }

static void scan_dir(const char *d)
{
    DIR *dd = opendir(d);
    if (!dd) return;
    struct dirent *e;
    while ((e = readdir(dd)) && nfiles < MAXF) {
        if (e->d_name[0] == '.') continue;
        char p[1100]; snprintf(p, sizeof p, "%s/%s", d, e->d_name);
        struct stat st;
        if (stat(p, &st) || !S_ISREG(st.st_mode) || !rom_size((long)st.st_size)) continue;
        mt32emu_rom_info ri;
        if (mt32emu_identify_rom_file(&ri, p, NULL) != MT32EMU_RC_OK || (!ri.control_rom_id && !ri.pcm_rom_id)) continue;
        RomFile *f = &files[nfiles++];
        snprintf(f->path, sizeof f->path, "%s", p);
        f->ctrl = ri.control_rom_id; f->pcm = ri.pcm_rom_id;
    }
    closedir(dd);
}

static int in_list(const char *id, const char **ids, size_t n)
{
    if (!id) return 0;
    for (size_t i = 0; i < n; i++) if (!strcmp(ids[i], id)) return 1;
    return 0;
}

/* the machines in order of preference: the MT-32s (the older first), or the CM-32L / LAPC-I first
 * when the player chose it, then whatever else the library knows */
static int machine_rank(const char *m)
{
    static const char *pref[] = { "cm32l_1_02", "cm32l_1_00", "cm32ln_1_00", "mt32_1_07", "mt32_1_06", "mt32_1_05", "mt32_1_04",
                                  "mt32_bluer", "mt32_2_04", "mt32_2_07", "mt32_2_06", "mt32_2_03" };
    int mt_first = !strcmp(config_get("sound.mt32_model", "mt32"), "mt32");     /* the MT-32 first (the default), or the CM-32L */
    for (size_t i = 0; i < sizeof pref / sizeof pref[0]; i++)
        if (!strcmp(pref[i], m)) return (int)i + (mt_first && !strncmp(m, "cm32l", 5) ? 50 : 0);
    return 100;
}
static int families;                    /* 1: a CM-32L pair was found, 2: an MT-32 pair */
static void scan(void);
int mt32_families(void) { if (!scanned) scan(); return families; }

/* which machine the files found make up; 1 if one does */
static int choose(void)
{
    const char *ms[64]; size_t nm = mt32emu_get_machine_ids(ms, 64);
    if (nm > 64) nm = 64;
    int best = -1, bestrank = 1000;
    for (size_t k = 0; k < nm; k++) {
        const char *ids[256]; size_t ni = mt32emu_get_rom_ids(ids, 256, ms[k]);
        if (ni > 256) ni = 256;
        int c = 0, p = 0;
        for (int i = 0; i < nfiles; i++) { c |= in_list(files[i].ctrl, ids, ni); p |= in_list(files[i].pcm, ids, ni); }
        if (c && p) families |= !strncmp(ms[k], "cm32l", 5) ? 1 : 2;
        if (c && p && machine_rank(ms[k]) < bestrank) { best = (int)k; bestrank = machine_rank(ms[k]); }
    }
    if (best < 0) return 0;
    snprintf(machine, sizeof machine, "%s", ms[best]);
    return 1;
}

static void scan(void)
{
    scanned = 1; nfiles = 0; machine[0] = 0; found_dir[0] = 0; families = 0;
    mt32_dirs();
    for (int i = 0; i < ndirs; i++) {
        int before = nfiles;
        scan_dir(dirs[i]);
        if (nfiles > before && choose()) { snprintf(found_dir, sizeof found_dir, "%s", dirs[i]); break; }
    }
    if (!machine[0] && nfiles) choose();          /* the two halves in different folders */
    fprintf(stderr, "mt32: %d ROM file%s found%s%s\n", nfiles, nfiles == 1 ? "" : "s", machine[0] ? ", machine " : "", machine);
}

/* 1 if a usable pair of ROMs is there (looked for once; mt32_rescan() looks again) */
int mt32_available(void)
{
    if (!scanned) scan();
    return machine[0] != 0;
}
void mt32_rescan(void) { scanned = 0; mt32_available(); if (state < 0) { state = 0; why[0] = 0; } }   /* a failed start may be tried again */

/* ---------------------------------------------------------------- the synthesizer */
/* its reports: quiet, but for the message the game puts on the synthesizer's display */
static mt32emu_report_handler_version MT32EMU_C_CALL rh_version(mt32emu_report_handler_i i) { (void)i; return MT32EMU_REPORT_HANDLER_VERSION_0; }
static void MT32EMU_C_CALL rh_debug(void *d, const char *fmt, va_list l) { (void)d; if (getenv("DL_MT32_LOG")) { vfprintf(stderr, fmt, l); fputc('\n', stderr); } }
static void MT32EMU_C_CALL rh_lcd(void *d, const char *m)
{
    (void)d; static char last[64];
    if (strncmp(last, m, sizeof last - 1)) {
        snprintf(last, sizeof last, "%s", m); fprintf(stderr, "mt32 display: %s\n", m);
        void ui_mt32_lcd(const char *text); ui_mt32_lcd(m);
    }
}
static const mt32emu_report_handler_i_v0 RH = { rh_version, rh_debug, NULL, NULL, rh_lcd };

static int out_rate;

/* a synthesizer of the machine found, or 0 (why[] says why) */
static mt32emu_context make_synth(int rate)
{
    mt32emu_report_handler_i rh; rh.v0 = &RH;
    mt32emu_context c = mt32emu_create_context(rh, NULL);
    for (int i = 0; i < nfiles; i++) mt32emu_add_machine_rom_file(c, machine, files[i].path);
    mt32emu_rom_info ri; mt32emu_get_rom_info(c, &ri);
    if (!ri.control_rom_id || !ri.pcm_rom_id) { snprintf(why, sizeof why, "the ROM files do not make a pair"); mt32emu_free_context(c); return 0; }
    mt32emu_set_stereo_output_samplerate(c, rate);
    mt32emu_set_samplerate_conversion_quality(c, MT32EMU_SRCQ_GOOD);
    mt32emu_set_analog_output_mode(c, MT32EMU_AOM_ACCURATE);
    if (mt32emu_open_synth(c) != MT32EMU_RC_OK) { snprintf(why, sizeof why, "these ROM files did not work"); mt32emu_free_context(c); return 0; }
    mt32emu_set_midi_event_queue_size(c, 8192);              /* (room for a burst: a song's timbres, several songs at once) */
    snprintf(found_desc, sizeof found_desc, "%s, %s", ri.control_rom_description ? ri.control_rom_description : ri.control_rom_id,
             ri.pcm_rom_description ? ri.pcm_rom_description : ri.pcm_rom_id);
    snprintf(playing, sizeof playing, "%s", machine);
    fprintf(stderr, "mt32: %s (%s)\n", found_desc, found_dir);
    return c;
}

int mt32_open(int rate)
{
    if (state) return state > 0;
    state = -1;
    if (!mt32_available()) { snprintf(why, sizeof why, "no MT-32 or CM-32L ROM files found"); return 0; }
    if (!(ctx = make_synth(rate))) return 0;
    out_rate = rate;
    {   /* say so once, the first time it plays */
        void ui_notice(const char *msg);
        if (!config_get_int("sound.mt32_told", 0)) {
            const char *m = "The music now plays as on a Roland MT-32, the 1987 synthesizer it was written for (F12 > Settings > Sound)";
            void ui_mt32_caption(const char *text);
            if (config_get_int("sound.mt32_lcd", 1)) ui_mt32_caption(m);   /* under its display, which shows the game's greeting */
            else ui_notice(m);
            config_set_int("sound.mt32_told", 1);
        }
    }
    state = 1;
    return 1;
}

int mt32_is_open(void) { return state > 0; }

/* Timing: the audio is made a buffer at a time, so a message would otherwise wait for the next
 * buffer and the notes of one buffer's time would all start together. Each message is given the
 * moment it was sent instead, one buffer later than now (the time since the last buffer began,
 * added to where that buffer began in the output, plus a buffer): an even delay, no jitter. */
uint64_t plat_ticks_us(void);
static uint64_t cb_us;                  /* when the last buffer was made */
static uint32_t rendered, cb_frames;    /* output frames made so far (that buffer included), and its length */
static uint32_t last_ts;

/* What the game has set up in the synthesizer, so that another machine can take over in the
 * middle of the game (F12 > Settings): the SysEx (its patches, timbres and settings; a
 * later one to the same address replaces an earlier one) and each channel's program, controllers
 * and pitch bend. */
typedef struct { unsigned char *b; int n; } Sx;
static Sx *sxs; static int nsx, capsx;
static short prog[16] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
static short ctl[16][128], bend[16];
static unsigned char ctl_set[16][128];
static void remember(const unsigned char *m, int n)
{
    int ch = m[0] & 15;
    switch (m[0] & 0xf0) {
    case 0xc0: if (n > 1) prog[ch] = m[1]; return;
    case 0xb0: if (n > 2 && m[1] < 120) { ctl[ch][m[1]] = m[2]; ctl_set[ch][m[1]] = 1; } return;
    case 0xe0: if (n > 2) bend[ch] = (short)(m[1] | m[2] << 8 | 0x8000); return;
    }
    if (m[0] != 0xf0 || n < 9 || n > 4096) return;
    /* F0 41 dev model 12 a a a data.. sum F7: the same address and length replace */
    for (int i = 0; i < nsx; i++)
        if (sxs[i].n == n && !memcmp(sxs[i].b + 1, m + 1, 7)) {
            unsigned char *b = sxs[i].b; memmove(&sxs[i], &sxs[i + 1], (size_t)(nsx - i - 1) * sizeof *sxs);
            memcpy(b, m, (size_t)n); sxs[nsx - 1].b = b; sxs[nsx - 1].n = n;
            return;
        }
    if (nsx >= 2048) return;
    if (nsx == capsx) { capsx = capsx ? capsx * 2 : 64; sxs = realloc(sxs, (size_t)capsx * sizeof *sxs); }
    sxs[nsx].b = malloc((size_t)n); memcpy(sxs[nsx].b, m, (size_t)n); sxs[nsx].n = n; nsx++;
}
static void replay(mt32emu_context c)
{
    for (int i = 0, tries = 0; i < nsx; i++)
        if (mt32emu_play_sysex(c, sxs[i].b, (mt32emu_bit32u)sxs[i].n) != MT32EMU_RC_OK && ++tries < 200) {   /* the queue full: let it take them in */
            static short junk[2 * 64]; mt32emu_render_bit16s(c, junk, 64); i--;
        }
    for (int ch = 0; ch < 16; ch++) {
        if (prog[ch] >= 0) mt32emu_play_msg(c, (mt32emu_bit32u)(0xc0 | ch) | (mt32emu_bit32u)prog[ch] << 8);
        for (int k = 0; k < 120; k++)
            if (ctl_set[ch][k]) mt32emu_play_msg(c, (mt32emu_bit32u)(0xb0 | ch) | (mt32emu_bit32u)k << 8 | (mt32emu_bit32u)ctl[ch][k] << 16);
        if (bend[ch] & 0x8000) mt32emu_play_msg(c, (mt32emu_bit32u)(0xe0 | ch) | (mt32emu_bit32u)(bend[ch] & 0x7f7f) << 8);
    }
}

/* from the game's thread: a whole MIDI message or SysEx (mpu.c) */
static unsigned full;                         /* (messages the queue had no room for) */
void mt32_msg(const unsigned char *m, int n)
{
    if (state <= 0) return;
    sound_lock(1);
    extern volatile uint32_t sound_tick_late_us;           /* (runtime/bios.c: the timer tick being delivered was late by so much; dated back by it) */
    int64_t dt = (int64_t)(plat_ticks_us() - cb_us) - (int64_t)sound_tick_late_us + 8000;   /* (8 ms more than a buffer later: room to date back a late tick) */
    if (dt < 0) dt = 0;
    if (dt > 2 * 1000000ll * cb_frames / 44100) dt = 2 * 1000000ll * cb_frames / 44100;   /* after a pause */
    uint32_t ts = mt32emu_convert_output_to_synth_timestamp(ctx, rendered + (uint32_t)(dt * 44100 / 1000000));
    if ((int32_t)(ts - last_ts) < 0) ts = last_ts;
    last_ts = ts;
    { static int lg = -1; if (lg < 0) lg = getenv("DL_MIDI_LOG") != 0;    /* (a test: each note's tick and where it was placed, for the timing) */
      if (lg && (m[0] & 0xf0) == 0x90 && n > 2 && m[2]) { extern volatile uint32_t music_ticks; fprintf(stderr, "midi note %u %u %u %llu ch %d key %d\n", music_ticks, ts, (unsigned)(plat_ticks_us() / 1000), (unsigned long long)(plat_ticks_us() - sound_tick_late_us), (m[0] & 15) + 1, m[1]); } }
    /* the queue full (a burst: several songs started at once, each uploading its timbres): the
       message goes in at once rather than being lost (a lost timbre upload would leave an
       instrument wrong for the session). (Safe here: the lock keeps the renderer out.) */
    mt32emu_bit32u sm = (mt32emu_bit32u)m[0] | (n > 1 ? (mt32emu_bit32u)m[1] << 8 : 0) | (n > 2 ? (mt32emu_bit32u)m[2] << 16 : 0);
    unsigned was = full;
    if (m[0] == 0xf0) { if (mt32emu_play_sysex_at(ctx, m, (mt32emu_bit32u)n, ts) != MT32EMU_RC_OK) { full++; mt32emu_play_sysex_now(ctx, m, (mt32emu_bit32u)n); } }
    else if (mt32emu_play_msg_at(ctx, sm, ts) != MT32EMU_RC_OK) { full++; mt32emu_play_msg_now(ctx, sm); }
    if (full != was && (full == 1 || full % 500 == 0)) fprintf(stderr, "mt32: the MIDI queue was full (%u times): played at once\n", full);
    sound_lock(0);
    remember(m, n);
}

/* from the audio callback (sound.c): add the synthesizer's output to the stereo stream */
void mt32_mix(short *out, int frames)
{
    if (state <= 0) return;
    cb_us = plat_ticks_us(); cb_frames = (uint32_t)frames;
    static short buf[2 * 4096];
    while (frames > 0) {
        int n = frames > 4096 ? 4096 : frames;
        mt32emu_render_bit16s(ctx, buf, (mt32emu_bit32u)n);
        for (int i = 0; i < 2 * n; i++) {
            int y = out[i] + buf[i];
            out[i] = (short)(y > 32767 ? 32767 : y < -32768 ? -32768 : y);
        }
        out += 2 * n; frames -= n;
    }
    rendered += cb_frames;
}

static const char *machine_desc(const char *m)
{
    static char b[64];
    if (!strncmp(m, "cm32ln", 6)) return "CM-32LN";
    if (!strncmp(m, "cm32l", 5)) return "CM-32L / LAPC-I";
    if (!strncmp(m, "mt32_", 5)) {
        const char *v = m + 5;
        if (!strcmp(v, "bluer")) return "MT-32 (Blue Ridge)";
        snprintf(b, sizeof b, "MT-32, version %c.%s", v[0], v + 2);
        return b;
    }
    return m;
}

/* for F12 > Settings */
const char *mt32_status(void)
{
    static char s[1600];
    if (state > 0) snprintf(s, sizeof s, "Playing as on the %s (%s; from %s).", machine_desc(playing), found_desc, found_dir);
    else if (mt32_available()) snprintf(s, sizeof s, "Found the ROM files of the %s in %s.", machine_desc(machine), found_dir[0] ? found_dir : "several folders");
    else if (state < 0 && why[0] && !strstr(why, "no MT-32")) snprintf(s, sizeof s, "The MT-32 could not start: %s.", why);
    else snprintf(s, sizeof s, "No MT-32 or CM-32L ROM files found. Put the two files (control and PCM, any names) into %s/roms, "
                  "or name their folder here.", dlp_user_dir[0] ? dlp_user_dir : game_dir);
    return s;
}
/* 1 if the ROMs found now make another machine than the one playing */
int mt32_switch_pending(void) { return state > 0 && mt32_available() && strcmp(playing, machine) != 0; }

/* from the game's thread: that machine takes over, with what the game set up in the old one */
int mt32_switch(void)
{
    if (!mt32_switch_pending()) return 0;
    mt32emu_context c = make_synth(out_rate);
    if (!c) return 0;
    replay(c);
    sound_lock(1);
    mt32emu_context old = ctx;
    ctx = c; rendered = 0; last_ts = 0;
    sound_lock(0);
    mt32emu_close_synth(old); mt32emu_free_context(old);
    fprintf(stderr, "mt32: switched to %s, %d SysEx replayed\n", machine, nsx);
    return 1;
}
int mt32_dir_count(void) { if (!ndirs) mt32_dirs(); return ndirs; }
const char *mt32_dir(int i) { return i >= 0 && i < ndirs ? dirs[i] : ""; }
