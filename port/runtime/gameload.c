/* gameload.c - the program without the generated code (DL_RTSPLIT, see split.h) loads the game
 * module the player's machine made from DARKLAND.EXE: darklands_game.dll (Windows) or
 * libdarklands_game.so, in the game folder, or where DL_GAME_MODULE says. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "hle.h"
#include "hooks.h"

#ifdef DL_RTSPLIT
#ifdef _WIN32
#include <windows.h>
#define MODULE_NAME "darklands_game.dll"
#else
#include <dlfcn.h>
#define MODULE_NAME "libdarklands_game.so"
#endif

const DlGame *dl_game;
extern volatile int poll_flag;
extern int hook_bypass;
extern u16 LOADSEG;
void interp_step(u16 cs, u16 ip);

extern const char dlp_translator[];
void gameload_path(const char *dir, char *path, int n)
{
    const char *env = getenv("DL_GAME_MODULE");
    if (env) snprintf(path, n, "%s", env);
    else snprintf(path, n, "%s/%s", dir, MODULE_NAME);
}

/* 1 loaded; 0 no module there, or one made by another translator (the caller makes one);
   -1 a module that cannot be used */
int gameload(const char *dir, char *err, int errn)
{
    char path[1200];
    gameload_path(dir, path, sizeof path);
    typedef const DlGame *(*InitFn)(const DlRuntime *);
    InitFn init = 0;
#ifdef _WIN32
    if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) { snprintf(err, errn, "%s: not made yet", path); return 0; }
    HMODULE h = LoadLibraryA(path);
    if (!h) { snprintf(err, errn, "%s: cannot load (error %lu)", path, (unsigned long)GetLastError()); return -1; }
    init = (InitFn)(void (*)(void))GetProcAddress(h, "dl_game_init");
#else
    FILE *t = fopen(path, "rb");
    if (!t) { snprintf(err, errn, "%s: not made yet", path); return 0; }
    fclose(t);
    void *h = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!h) { snprintf(err, errn, "%s", dlerror()); return -1; }
    init = (InitFn)dlsym(h, "dl_game_init");
#endif
    if (!init) {
        snprintf(err, errn, "%s: not a Darklands+ game module", path);
#ifdef _WIN32
        FreeLibrary(h);                 /* so that it can be made again in its place */
#else
        dlclose(h);
#endif
        return -1;
    }
    static DlRuntime rt;
    rt.version = DL_SPLIT_VERSION;
    rt.cpu_p = &R; rt.mem_p = M; rt.loadseg_p = &LOADSEG;
    rt.tail_pending_p = &tail_pending; rt.tail_sp_p = &tail_sp;
    rt.unw_active_p = &unw_active; rt.unw_sp_p = &unw_sp; rt.unw_ip_p = &unw_ip; rt.unw_cs_p = &unw_cs;
    rt.hook_bypass_p = &hook_bypass; rt.poll_flag_p = &poll_flag;
    rt.dispatch_far = dispatch_far; rt.dispatch_near = dispatch_near;
    rt.dispatch_jmp_far = dispatch_jmp_far; rt.dispatch_jmp_near = dispatch_jmp_near;
    rt.dl_poll = dl_poll; rt.fatal = fatal;
    rt.hle_in8 = hle_in8; rt.hle_out8 = hle_out8; rt.hle_int = hle_int;
    rt.hook_run = hook_run; rt.interp_run = interp_run; rt.interp_step = interp_step;
    rt.ovl_enter = ovl_enter; rt.ret_mismatch = ret_mismatch;
    dl_game = init(&rt);
    if (!dl_game || strcmp(dl_game->translator, dlp_translator)) {
        if (getenv("DL_GAME_MODULE") && dl_game) { fprintf(stderr, "game module %s: made by translator %s, not %s (used anyway)\n", path, dl_game->translator, dlp_translator); return 1; }
        snprintf(err, errn, "%s: made by an earlier Darklands+ (translator %s, now %s)", path, dl_game ? dl_game->translator : "?", dlp_translator);
        dl_game = 0;
#ifdef _WIN32
        FreeLibrary(h);
#else
        dlclose(h);
#endif
        return 0;
    }
    fprintf(stderr, "game module %s (translator %s)\n", path, dl_game->translator);
    return 1;
}
#endif
