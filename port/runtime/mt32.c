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
 * mt32_dirs(): the folder set in F12 > Display, "roms" folders of Darklands+ and of the game,
 * Munt's own ROM folder (its registry setting, or %USERPROFILE%\roms) and DOSBox Staging's
 * mt32-roms folder. A CM-32L / LAPC-I pair is preferred (the LAPC-I is what the game names; it
 * also has the extra sound effects), then the MT-32's. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
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

/* the machines in order of preference: CM-32L / LAPC-I first, then the MT-32s (the older first),
 * then whatever else the library knows */
static int machine_rank(const char *m)
{
    static const char *pref[] = { "cm32l_1_02", "cm32l_1_00", "cm32ln_1_00", "mt32_1_07", "mt32_1_06", "mt32_1_05", "mt32_1_04",
                                  "mt32_bluer", "mt32_2_04", "mt32_2_07", "mt32_2_06", "mt32_2_03" };
    for (size_t i = 0; i < sizeof pref / sizeof pref[0]; i++) if (!strcmp(pref[i], m)) return (int)i;
    return 100;
}

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
        if (c && p && machine_rank(ms[k]) < bestrank) { best = (int)k; bestrank = machine_rank(ms[k]); }
    }
    if (best < 0) return 0;
    snprintf(machine, sizeof machine, "%s", ms[best]);
    return 1;
}

static void scan(void)
{
    scanned = 1; nfiles = 0; machine[0] = 0; found_dir[0] = 0;
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
void mt32_rescan(void) { scanned = 0; mt32_available(); }

/* ---------------------------------------------------------------- the synthesizer */
/* its reports: quiet, but for the message the game puts on the synthesizer's display */
static mt32emu_report_handler_version MT32EMU_C_CALL rh_version(mt32emu_report_handler_i i) { (void)i; return MT32EMU_REPORT_HANDLER_VERSION_0; }
static void MT32EMU_C_CALL rh_debug(void *d, const char *fmt, va_list l) { (void)d; if (getenv("DL_MT32_LOG")) { vfprintf(stderr, fmt, l); fputc('\n', stderr); } }
static void MT32EMU_C_CALL rh_lcd(void *d, const char *m)
{
    (void)d; static char last[64];
    if (strncmp(last, m, sizeof last - 1)) { snprintf(last, sizeof last, "%s", m); fprintf(stderr, "mt32 display: %s\n", m); }
}
static const mt32emu_report_handler_i_v0 RH = { rh_version, rh_debug, NULL, NULL, rh_lcd };

int mt32_open(int rate)
{
    if (state) return state > 0;
    state = -1;
    if (!mt32_available()) { snprintf(why, sizeof why, "no MT-32 or CM-32L ROM files found"); return 0; }
    mt32emu_report_handler_i rh; rh.v0 = &RH;
    ctx = mt32emu_create_context(rh, NULL);
    for (int i = 0; i < nfiles; i++) mt32emu_add_machine_rom_file(ctx, machine, files[i].path);
    mt32emu_rom_info ri; mt32emu_get_rom_info(ctx, &ri);
    if (!ri.control_rom_id || !ri.pcm_rom_id) { snprintf(why, sizeof why, "the ROM files do not make a pair"); mt32emu_free_context(ctx); ctx = 0; return 0; }
    mt32emu_set_stereo_output_samplerate(ctx, rate);
    mt32emu_set_samplerate_conversion_quality(ctx, MT32EMU_SRCQ_GOOD);
    mt32emu_set_analog_output_mode(ctx, MT32EMU_AOM_ACCURATE);
    if (mt32emu_open_synth(ctx) != MT32EMU_RC_OK) { snprintf(why, sizeof why, "the emulator did not start"); mt32emu_free_context(ctx); ctx = 0; return 0; }
    snprintf(found_desc, sizeof found_desc, "%s, %s", ri.control_rom_description ? ri.control_rom_description : ri.control_rom_id,
             ri.pcm_rom_description ? ri.pcm_rom_description : ri.pcm_rom_id);
    fprintf(stderr, "mt32: %s (%s)\n", found_desc, found_dir);
    state = 1;
    return 1;
}

int mt32_is_open(void) { return state > 0; }

/* from the game's thread: a whole MIDI message or SysEx (mpu.c) */
void mt32_msg(const unsigned char *m, int n)
{
    if (state <= 0) return;
    sound_lock(1);
    if (m[0] == 0xf0) mt32emu_play_sysex(ctx, m, (mt32emu_bit32u)n);
    else mt32emu_play_msg(ctx, (mt32emu_bit32u)m[0] | (n > 1 ? (mt32emu_bit32u)m[1] << 8 : 0) | (n > 2 ? (mt32emu_bit32u)m[2] << 16 : 0));
    sound_lock(0);
}

/* from the audio callback (sound.c): add the synthesizer's output to the stereo stream */
void mt32_mix(short *out, int frames)
{
    if (state <= 0) return;
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
}

/* for F12 > Display */
const char *mt32_status(void)
{
    static char s[1600];
    if (state > 0) snprintf(s, sizeof s, "Playing on %s (from %s).", found_desc, found_dir);
    else if (mt32_available()) snprintf(s, sizeof s, "ROM files found (%s, in %s).", machine, found_dir[0] ? found_dir : "several folders");
    else if (state < 0 && why[0] && !strstr(why, "no MT-32")) snprintf(s, sizeof s, "The MT-32 could not start: %s.", why);
    else snprintf(s, sizeof s, "No MT-32 or CM-32L ROM files found. Put the two files (control and PCM, any names) into %s/roms, "
                  "or name their folder here.", dlp_user_dir[0] ? dlp_user_dir : game_dir);
    return s;
}
int mt32_dir_count(void) { if (!ndirs) mt32_dirs(); return ndirs; }
const char *mt32_dir(int i) { return i >= 0 && i < ndirs ? dirs[i] : ""; }
