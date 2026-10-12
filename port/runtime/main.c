/* main.c - start-up of the native Darklands: load the original DARKLAND.EXE image into the
 * emulated address space (the recompiled code reads its data, code-segment tables and
 * overlay sections from there), set up DOS structures, and run the program. */
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "hle.h"
#ifndef DL_RTSPLIT
#include "funcs.h"
#endif
#include <SDL.h>
#include "config.h"
#include "../mods/mods.h"
void dos_mem_init2(u16 env, u16 env_paras, u16 psp);



u16 LOADSEG = 0x0200;
extern int dos_trace, bios_trace;

static int file_exists(const char *p) { FILE *f = fopen(p, "rb"); if (f) { fclose(f); return 1; } return 0; }

char dlp_data_dir[1100];         /* Darklands+'s own files: the game folder, or %LOCALAPPDATA%\\Darklands+ */
char dlp_user_dir[1100];         /* the player's mods, art and sounds folders: the game folder, or the data folder when it cannot be written */
static int dir_writable(const char *d)
{
    if (getenv("DL_READONLY_GAME")) return 0;     /* tests: as if the game folder could not be written */
    char p[1200]; snprintf(p, sizeof p, "%s/.darklands_plus_write_test", d);
    FILE *f = fopen(p, "wb");
    if (!f) return 0;
    fclose(f); remove(p);
    return 1;
}
char dlp_prog_dir[1100];         /* where darklands.exe is (the mods shipped with it: mods\ there) */
static int find_game_dir(int argc, char **argv)
{
    char p[1100];
    { char *b = SDL_GetBasePath();
      if (b) { snprintf(dlp_prog_dir, sizeof dlp_prog_dir, "%s", b); SDL_free(b);
               size_t n = strlen(dlp_prog_dir); if (n && (dlp_prog_dir[n - 1] == '/' || dlp_prog_dir[n - 1] == '\\')) dlp_prog_dir[n - 1] = 0; } }
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') continue;
        snprintf(game_dir, sizeof game_dir, "%s", argv[i]);
        snprintf(p, sizeof p, "%s/DARKLAND.EXE", game_dir);
        if (file_exists(p)) return 1;
        fprintf(stderr, "no DARKLAND.EXE in %s\n", game_dir);
        return 0;
    }
    /* next to the executable, then the current directory */
    const char *cands[4] = { 0, ".", 0, 0 };
    char sub[1100] = "";
    char exedir[1024] = "";
    {
        char *b = SDL_GetBasePath();
        if (b) { snprintf(exedir, sizeof exedir, "%s", b); SDL_free(b);
                 size_t n = strlen(exedir); if (n && (exedir[n - 1] == '/' || exedir[n - 1] == '\\')) exedir[n - 1] = 0;
                 cands[0] = exedir; snprintf(sub, sizeof sub, "%s/DARKLAND", exedir); cands[2] = sub; }
    }
    for (int i = 0; i < 3; i++) {
        if (!cands[i]) continue;
        snprintf(p, sizeof p, "%s/DARKLAND.EXE", cands[i]);
        if (file_exists(p)) { snprintf(game_dir, sizeof game_dir, "%s", cands[i]); return 1; }
    }
    /* elsewhere: a folder chosen before, the GOG / Steam installation, a folder dialog (gamefind.c) */
    int find_game_more(const char *exedir, char *out, int n, int interactive);
    if (exedir[0] && find_game_more(exedir, game_dir, sizeof game_dir, !getenv("DL_SCRIPT") && !getenv("DL_CTL") && !getenv("DL_HEADLESS"))) return 1;
    return 0;
}

/* the DARKLAND.EXE Darklands+ is made for: 483.07 as GOG and Steam sell it */
#define GAME_EXE_CRC 0x35aa8edeu
static unsigned file_crc32(const char *path)
{
    static unsigned t[256];
    if (!t[1]) for (unsigned i = 0; i < 256; i++) { unsigned c = i; for (int k = 0; k < 8; k++) c = c & 1 ? 0xedb88320u ^ (c >> 1) : c >> 1; t[i] = c; }
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    unsigned c = 0xffffffffu;
    unsigned char buf[65536]; size_t n;
    while ((n = fread(buf, 1, sizeof buf, f)) > 0) for (size_t i = 0; i < n; i++) c = t[(c ^ buf[i]) & 0xff] ^ (c >> 8);
    fclose(f);
    return c ^ 0xffffffffu;
}

static u16 le16(const u8 *p) { return (u16)(p[0] | (p[1] << 8)); }

/* a crash of the program itself (not of the game it runs): where, in the log, so a report
 * can say; on Linux with the native call stack, on Windows with the address in the program */
#ifdef _WIN32
#include <windows.h>
static LONG WINAPI crash_filter(EXCEPTION_POINTERS *e)
{
    HMODULE base = GetModuleHandleA(0);
    fprintf(stderr, "CRASH: exception %08lx at %p (program base %p, offset %llx); game code segment %04x\n",
            e->ExceptionRecord->ExceptionCode, e->ExceptionRecord->ExceptionAddress, (void *)base,
            (unsigned long long)((char *)e->ExceptionRecord->ExceptionAddress - (char *)base), R.cs);
    fflush(stderr);
    void plat_error_box(const char *msg);
    plat_error_box("The program crashed.");
    return EXCEPTION_EXECUTE_HANDLER;
}
static void crash_handler_install(void) { SetUnhandledExceptionFilter(crash_filter); }
#else
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
static void crash_signal(int sig)
{
    char b[160]; int n = snprintf(b, sizeof b, "CRASH: signal %d; game code segment %04x; native stack:\n", sig, R.cs);
    if (write(2, b, (size_t)n) < 0) { }
    void *fr[48]; int k = backtrace(fr, 48);
    backtrace_symbols_fd(fr, k, 2);
    signal(sig, SIG_DFL); raise(sig);
}
static void crash_handler_install(void)
{
    if (getenv("DL_NO_CRASH_HANDLER")) return;
    signal(SIGSEGV, crash_signal); signal(SIGBUS, crash_signal); signal(SIGFPE, crash_signal); signal(SIGILL, crash_signal); signal(SIGABRT, crash_signal);
}
#endif

int main_argc; char **main_argv;      /* for plat_restart */
int main(int argc, char **argv)
{
    main_argc = argc; main_argv = argv;
    int skip_rtlink = 1;
    if (getenv("DL_TRACE_DOS")) dos_trace = 1;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--trace-dos")) dos_trace = 1;
        else if (!strcmp(argv[i], "--trace-bios")) bios_trace = 1;
        else if (!strcmp(argv[i], "--rtlink")) skip_rtlink = 0;
    }
    if (!find_game_dir(argc, argv)) {
        const char *msg = "Darklands+ could not find your Darklands folder (the one that contains DARKLAND.EXE).\n"
                          "Put darklands.exe and the darklands_plus folder into it, or start darklands.exe with that folder as its argument.";
        fprintf(stderr, "%s\n", msg);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Darklands", msg, 0);
        return 1;
    }
    /* Darklands+'s own files (log, settings, the game module) go next to the game; where that
       folder cannot be written (Windows: under Program Files), into %LOCALAPPDATA%\Darklands+ */
#ifdef _WIN32
    {   /* one spelling of the folder (full, long names, backslashes): the shadow folder below
           must not depend on how the game was started */
        char full[1100], lng[1100];
        if (GetFullPathNameA(game_dir, sizeof full, full, NULL)) {
            if (!GetLongPathNameA(full, lng, sizeof lng)) snprintf(lng, sizeof lng, "%s", full);
            for (char *q = lng; *q; q++) if (*q == '/') *q = '\\';
            size_t l = strlen(lng); while (l > 3 && lng[l - 1] == '\\') lng[--l] = 0;
            snprintf(game_dir, sizeof game_dir, "%s", lng);
        }
    }
#endif
    int game_writable = dir_writable(game_dir);
    snprintf(dlp_data_dir, sizeof dlp_data_dir, "%s", game_dir);
    char shadow[1100] = "";                /* where the game's own writes go then (runtime/dos.c) */
#ifdef _WIN32
    {   /* under Program Files: Windows' VirtualStore copy of the folder, where Windows redirects
           old programs' writes (DOSBox may have saved there) - used whenever it exists, so that
           a start "as administrator" (the folder then writable) sees the same saves */
        char vs[1100] = "";
        const char *pf[] = { getenv("ProgramFiles"), getenv("ProgramFiles(x86)"), getenv("ProgramW6432"), getenv("SystemRoot") };
        for (int i = 0; i < 4 && !vs[0] && getenv("LOCALAPPDATA"); i++) {
            size_t l = pf[i] ? strlen(pf[i]) : 0;
            if (l && !_strnicmp(game_dir, pf[i], l) && game_dir[l] == '\\' && game_dir[1] == ':')
                snprintf(vs, sizeof vs, "%s\\VirtualStore%s", getenv("LOCALAPPDATA"), game_dir + 2);
        }
        DWORD at = vs[0] ? GetFileAttributesA(vs) : INVALID_FILE_ATTRIBUTES;
        if (at != INVALID_FILE_ATTRIBUTES && (at & FILE_ATTRIBUTE_DIRECTORY)) snprintf(shadow, sizeof shadow, "%s", vs);
        if (!game_writable && getenv("LOCALAPPDATA")) {
            snprintf(dlp_data_dir, sizeof dlp_data_dir, "%s\\Darklands+", getenv("LOCALAPPDATA"));
            CreateDirectoryA(dlp_data_dir, NULL);
            if (!shadow[0]) snprintf(shadow, sizeof shadow, "%s", vs[0] ? vs : "");
            if (!shadow[0]) snprintf(shadow, sizeof shadow, "%s\\game", dlp_data_dir);
        }
    }
#else
    if (!game_writable) {
        const char *x = getenv("XDG_DATA_HOME"), *h = getenv("HOME");
        if (x && *x) snprintf(dlp_data_dir, sizeof dlp_data_dir, "%s/darklands-plus", x);
        else if (h) snprintf(dlp_data_dir, sizeof dlp_data_dir, "%s/.local/share/darklands-plus", h);
        char t[1200]; snprintf(t, sizeof t, "%s/", dlp_data_dir);
        for (char *q = t + 1; *q; q++) if (*q == '/') { *q = 0; mkdir(t, 0755); *q = '/'; }
        snprintf(shadow, sizeof shadow, "%s/game", dlp_data_dir);
    }
#endif
#ifdef _WIN32
    {   /* no console on Windows: keep a log next to the game */
        char lp[1100]; snprintf(lp, sizeof lp, "%s/darklands_port.log", dlp_data_dir);
        freopen(lp, "w", stderr);
        setvbuf(stderr, NULL, _IONBF, 0);        /* every line at once: a crash loses nothing */
    }
#endif
    { extern const char dlp_version[]; fprintf(stderr, "Darklands+ build %s, game folder %s\n", dlp_version, game_dir); }
    snprintf(dlp_user_dir, sizeof dlp_user_dir, "%s", game_writable ? game_dir : dlp_data_dir);
    if (shadow[0]) {                       /* nothing to ask of the player: the game writes into the shadow folder */
        void dos_shadow_init(const char *dir);
        dos_shadow_init(shadow);
    }
    if (!game_writable) fprintf(stderr, "Darklands+'s own files (settings, log, the game module) are in %s\n", dlp_data_dir);
    crash_handler_install();
    char exe[1100];
    snprintf(exe, sizeof exe, "%s/DARKLAND.EXE", game_dir);
    {   unsigned crc = file_crc32(exe);
        if (crc != GAME_EXE_CRC) {
            char msg[600];
            snprintf(msg, sizeof msg, "This DARKLAND.EXE is not the one Darklands+ is made for (version 483.07, as GOG sells it; "
                     "this one's checksum is %08x). The game may not start, or may stop with an error.", crc);
            fprintf(stderr, "warning: %s\n", msg);
            if (!getenv("DL_SCRIPT") && !getenv("DL_CTL") && !getenv("DL_HEADLESS")) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Darklands", msg, 0);
        }
    }

    config_load(dlp_data_dir);
    {   /* SAVES\DEFAULT holds the ready-made world Quickstart begins with: without it Quickstart
           makes a party with no figures, and the first battle stops the game ("pics\short.pic") */
        char dp[1200]; struct stat st;
        snprintf(dp, sizeof dp, "%s/SAVES/DEFAULT", game_dir);
        if (stat(dp, &st) != 0) { snprintf(dp, sizeof dp, "%s/saves/default", game_dir); }
        if (stat(dp, &st) != 0) {
            const char *msg = "The file SAVES\\DEFAULT is missing from the game folder. Quickstart needs it (it holds the "
                              "ready-made party and world); without it the first battle stops the game. Copy it back from "
                              "your Darklands installation (GOG: the SAVES folder).";
            fprintf(stderr, "warning: %s\n", msg);
            if (!getenv("DL_SCRIPT") && !getenv("DL_CTL") && !getenv("DL_HEADLESS")) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Darklands", msg, 0);
        }
    }
    { void datamods_init(void); datamods_init(); }     /* mod folders (mods/datamods.c), before any file is read */
    M = calloc(1, MEM_SIZE);
#ifdef FTRACE
    { extern u32 watch_lo, watch_len; const char *w = getenv("DL_WATCH");   /* linear-hex[:len] */
      { extern u16 sp_low; const char *sl = getenv("DL_SP_LOW"); if (sl) sp_low = (u16)strtoul(sl, 0, 16); }
      const char *ir = getenv("DL_INTERP");      /* lo-hi[@sect] link addresses, hex */
      if (ir) { extern u32 interp_lo, interp_hi; extern int interp_sect; interp_lo = (u32)strtoul(ir, 0, 16);
                const char *d = strchr(ir, '-'); interp_hi = d ? (u32)strtoul(d + 1, 0, 16) : interp_lo + 1;
                const char *at = strchr(ir, '@'); if (at) interp_sect = atoi(at + 1); }
      if (w) { watch_lo = (u32)strtoul(w, 0, 16); const char *c = strchr(w, ':'); watch_len = c ? (u32)strtoul(c + 1, 0, 16) : 1; } }
#endif
    plat_init(1);
#ifdef DL_RTSPLIT
    {   /* the game's translated code, made on this machine from DARKLAND.EXE (split.h): made
           now if it is missing or an earlier Darklands+ made it */
        int gameload(const char *dir, char *err, int errn);
        void gameload_path(const char *dir, char *path, int n);
        int prepare_run(const char *game_dir, const char *module_path, char *err, int errn);
        char err[1400], mp[1200];
        int r = gameload(dlp_data_dir, err, sizeof err);
        if (r == 0 || (r < 0 && !getenv("DL_GAME_MODULE"))) {     /* missing, older, or damaged: made (again) */
            fprintf(stderr, "%s: making the game module\n", err);
            gameload_path(dlp_data_dir, mp, sizeof mp);
            if (prepare_run(game_dir, mp, err, sizeof err)) r = gameload(dlp_data_dir, err, sizeof err);
        }
        if (r <= 0) {
            char msg[1800]; snprintf(msg, sizeof msg, "Darklands+ could not get the game ready:\n%s%s", err,
#ifdef _WIN32
                                      "\n\nIf an antivirus program removed or blocked darklands_game.dll, allow that file "
                                      "(Darklands+ makes it from your own DARKLAND.EXE) and start again."
#else
                                      ""
#endif
                                      );
            fprintf(stderr, "%s\n", msg);
            if (!getenv("DL_SCRIPT") && !getenv("DL_CTL") && !getenv("DL_HEADLESS")) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Darklands", msg, 0);
            return 1;
        }
    }
#endif
    bios_init();

    /* ---- MZ load */
    FILE *f = fopen(exe, "rb");
    if (!f) fatal("cannot open %s", exe);
    u8 h[32];
    if (fread(h, 1, 32, f) != 32) fatal("short read");
    u32 hdr = le16(h + 8) * 16u;
    u32 size = (le16(h + 4) - 1) * 512u + (le16(h + 2) ? le16(h + 2) : 512) - hdr;
    u16 nrel = le16(h + 6), relo = le16(h + 0x18);
    u16 ss = le16(h + 0x0e), sp = le16(h + 0x10);
    fseek(f, hdr, SEEK_SET);
    if (fread(M + (u32)LOADSEG * 16, 1, size, f) != size) fatal("short read of load image");
    for (int k = 0; k < nrel; k++) {
        u8 r[4];
        fseek(f, relo + 4 * k, SEEK_SET);
        if (fread(r, 1, 4, f) != 4) fatal("short read");
        u32 a = ((u32)LOADSEG + le16(r + 2)) * 16u + le16(r);
        u16 v = (u16)(M[a] | (M[a + 1] << 8)) + LOADSEG;
        M[a] = (u8)v; M[a + 1] = (u8)(v >> 8);
    }
    fclose(f);

    /* ---- environment and PSP */
    u16 env = 0x0101;
    psp_seg = (u16)(LOADSEG - 0x10);
    {
        const char *vars[] = { "COMSPEC=C:\\COMMAND.COM", "PATH=C:\\", 0 };
        u32 a = (u32)env * 16;
        for (int i = 0; vars[i]; i++) { strcpy((char *)M + a, vars[i]); a += (u32)strlen(vars[i]) + 1; }
        M[a++] = 0;
        M[a++] = 1; M[a++] = 0;
        strcpy((char *)M + a, "C:\\DARKLAND.EXE");
    }
    {
        u32 p = (u32)psp_seg * 16;
        M[p] = 0xcd; M[p + 1] = 0x20;
        ww(psp_seg, 2, 0xa000);
        ww(psp_seg, 0x2c, env);
        M[p + 0x50] = 0xcd; M[p + 0x51] = 0x21; M[p + 0x52] = 0xcb;
        M[p + 0x80] = 0; M[p + 0x81] = 0x0d;
        for (int i = 0; i < 20; i++) M[p + 0x18 + i] = (u8)(i < 5 ? (i < 3 ? 1 : i) : 0xff);
        ww(psp_seg, 0x32, 20); ww(psp_seg, 0x34, 0x18); ww(psp_seg, 0x36, psp_seg);
    }
    dos_mem_init2(env, (u16)(psp_seg - 1 - env), psp_seg);

    { void ui_late_init(void); ui_late_init(); }
    mods_init();
    { void rules_apply(void); rules_apply(); }       /* mods' rules.ini (ui/rules.cpp) */

    /* ---- overlays */
    ovl_init(exe);
    ovl_preload();

    /* ---- registers as DOS leaves them */
    DS = ES = psp_seg;
    SS = (u16)(LOADSEG + ss); SP = sp;
    AX = BX = 0; CX = 0xff; DX = psp_seg; SI = 0; DI = SP; BP = 0x091c;
    set_flags(0x0202);
    R.iflag = 1;
    CS = (u16)(LOADSEG + 0x09c0);

    if (skip_rtlink) {
        /* what the RTLink start-up leaves behind: the PSP saved in its segment */
        ww((u16)(LOADSEG + 0x09c0), 2534, psp_seg);
#ifdef DL_RTSPLIT
        dl_game->start_c();
    } else {
        dl_game->start_rtlink();
#else
        F_root_06a2a();         /* C start-up (__astart) */
    } else {
        F_root_0a2fd();         /* RTLink start-up, which then runs __astart */
#endif
    }
    if (unw_active) fprintf(stderr, "program left through an unmatched return (slot %04x -> %04x:%04x)\n", unw_sp, unw_cs, unw_ip);
    else fprintf(stderr, "program returned from its entry point\n");
    plat_quit(0);
    return 0;
}
