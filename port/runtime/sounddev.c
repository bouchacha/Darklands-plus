/* sounddev.c - which sound driver the game gets. Darklands loads xSOUND.DLC (towns, map)
 * and xSOUND.DLB (battles), x from its configuration. The GOG release replaced the FM
 * (AdLib / Sound Blaster Pro / OPL-3) drivers with copies of the Roland ones and kept the FM
 * drivers as PSOUND.DC/.DB and ASOUND.DC/.DB, so on a modern machine the battle sounds come
 * out of a General MIDI synth "like random notes rather than clashing swords". Here the
 * drivers are recognised by their contents and the player's choice of device decides:
 *   fm      - OPL-3 FM (Sound Blaster Pro 2 / Pro Audio Spectrum 16), emulated (Nuked OPL3)
 *   adlib   - OPL-2 FM (AdLib / original Sound Blaster), emulated
 *   midi    - the Roland driver, MIDI to the system's synthesizer (or an MT-32 emulator)
 *   speaker - PC speaker
 *   off     - silence
 *   game    - whatever the game's configuration asks for (no substitution)
 * Setting "sound.device" in darklands_port.ini; default fm. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <ctype.h>
#include "cpu.h"
#include "config.h"
#ifdef _WIN32
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

extern char game_dir[1024];

enum { DEV_FM, DEV_ADLIB, DEV_MIDI, DEV_SPEAKER, NDEV };
static const char *SIG[NDEV][2] = {          /* signature strings: towns/map driver, battle driver */
    { "P Dark Cards", "P DarkBattle" },
    { "DrkCardAdLib", "DKBttleAdLib" },
    { "RLND DrkLand", "RLND DrkLand" },
    { "DarkCardsIBM", "DarkBattlIBM" },
};
static char found[NDEV][2][64];
static int scanned;

static int has(const unsigned char *b, size_t n, const char *s)
{
    size_t k = strlen(s);
    for (size_t i = 0; i + k <= n; i++) if (b[i] == (unsigned char)s[0] && !memcmp(b + i, s, k)) return 1;
    return 0;
}

static void scan(void)
{
    scanned = 1;
    DIR *d = opendir(game_dir);
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d))) {
        const char *n = e->d_name;
        if (strlen(n) < 9 || strncasecmp(n + 1, "SOUND.", 6)) continue;
        const char *ext = n + 7;
        char p[1200]; snprintf(p, sizeof p, "%s/%s", game_dir, n);
        FILE *f = fopen(p, "rb");
        if (!f) continue;
        static unsigned char buf[0x20000];
        size_t len = fread(buf, 1, sizeof buf, f);
        fclose(f);
        if (len < 2 || buf[0] != 'M' || buf[1] != 'Z') continue;
        for (int dev = 0; dev < NDEV; dev++)
            for (int role = 0; role < 2; role++) {
                if (!has(buf, len, SIG[dev][role])) continue;
                /* the Roland pair shares one signature: tell them apart by the extension */
                if (dev == DEV_MIDI && (toupper((unsigned char)ext[strlen(ext) - 1]) == 'B') != (role == 1)) continue;
                if (!found[dev][role][0] || toupper((unsigned char)n[0]) == "PARI"[dev])
                    snprintf(found[dev][role], sizeof found[dev][role], "%s", n);
            }
    }
    closedir(d);
    for (int dev = 0; dev < NDEV; dev++)
        fprintf(stderr, "sound drivers %s: %s / %s\n", (const char *[]){ "fm", "adlib", "midi", "speaker" }[dev],
                found[dev][0][0] ? found[dev][0] : "-", found[dev][1][0] ? found[dev][1] : "-");
}

int sound_device(void)
{
    if (getenv("DL_NOSOUND")) return -1;
    const char *c = config_get("sound.device", "fm");
    if (!strcmp(c, "fm")) return DEV_FM;
    if (!strcmp(c, "adlib")) return DEV_ADLIB;
    if (!strcmp(c, "midi")) return DEV_MIDI;
    if (!strcmp(c, "speaker")) return DEV_SPEAKER;
    if (!strcmp(c, "off")) return -1;
    return -2;                                   /* "game" */
}

/* the driver file to load for the game's request base name (xSOUND.DLC / .DLB), or 0 to
 * load what was asked for */
const char *sound_pick(const char *base)
{
    if (strlen(base) != 10 || strncasecmp(base + 1, "SOUND.DL", 8)) return 0;
    int role = toupper((unsigned char)base[9]) == 'B';
    int dev = sound_device();
    if (dev == -2) return 0;
    if (dev == -1) return role ? "NSOUND.DLB" : "NSOUND.DLC";      /* no battle driver: silence, as with the game's own "no sound" */
    if (!scanned) scan();
    int order[NDEV][NDEV] = { { DEV_FM, DEV_ADLIB, DEV_MIDI, DEV_SPEAKER }, { DEV_ADLIB, DEV_FM, DEV_MIDI, DEV_SPEAKER },
                              { DEV_MIDI, DEV_FM, DEV_ADLIB, DEV_SPEAKER }, { DEV_SPEAKER, DEV_FM, DEV_ADLIB, DEV_MIDI } };
    for (int k = 0; k < NDEV; k++) {
        int d = order[dev][k];
        if (found[d][role][0]) {
            if (strcasecmp(found[d][role], base)) fprintf(stderr, "sound: %s instead of %s\n", found[d][role], base);
            return found[d][role];
        }
    }
    return 0;
}
