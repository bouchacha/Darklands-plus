/* gameload.c - the program without the generated code (DL_RTSPLIT, see split.h) loads the game
 * module the player's machine made from DARKLAND.EXE: darklands_game.dll (Windows) or
 * libdarklands_game.so, in the game folder, or where DL_GAME_MODULE says. */
#include <stdio.h>
#include <stdlib.h>
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

const char *gameload_name(void) { return MODULE_NAME; }

/* 1 loaded; 0 no module there (the caller may make one); -1 a module that does not fit */
int gameload(const char *dir, char *err, int errn)
{
    char path[1200];
    const char *env = getenv("DL_GAME_MODULE");
    if (env) snprintf(path, sizeof path, "%s", env);
    else snprintf(path, sizeof path, "%s/%s", dir, MODULE_NAME);
    typedef const DlGame *(*InitFn)(const DlRuntime *);
    InitFn init = 0;
#ifdef _WIN32
    HMODULE h = LoadLibraryA(path);
    if (!h) { snprintf(err, errn, "%s: cannot load (error %lu)", path, (unsigned long)GetLastError()); return GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES ? 0 : -1; }
    init = (InitFn)(void (*)(void))GetProcAddress(h, "dl_game_init");
#else
    FILE *t = fopen(path, "rb");
    if (!t) { snprintf(err, errn, "%s: not there", path); return 0; }
    fclose(t);
    void *h = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!h) { snprintf(err, errn, "%s", dlerror()); return -1; }
    init = (InitFn)dlsym(h, "dl_game_init");
#endif
    if (!init) { snprintf(err, errn, "%s: not a Darklands+ game module", path); return -1; }
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
    if (!dl_game) { snprintf(err, errn, "%s: made for another version of Darklands+", path); return -1; }
    fprintf(stderr, "game module %s\n", path);
    return 1;
}
#endif
