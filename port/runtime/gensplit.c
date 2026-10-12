/* gensplit.c - the game module's side of split.h (compiled with DL_GENSPLIT, together with the
 * generated code, cpu.c and overrides.c): the runtime's variables are pointers set here, its
 * functions are reached through the stubs below, and dl_game_init hands the generated tables
 * back. */
#include <stdarg.h>
#include <stdio.h>
#include "cpu.h"
#include "hooks.h"
#include "dispatch.h"
#include "funcs.h"

#ifndef DL_GENSPLIT
#error gensplit.c belongs in the game module (DL_GENSPLIT)
#endif
#ifdef _WIN32
#define DLEXPORT __declspec(dllexport)
#else
#define DLEXPORT __attribute__((visibility("default")))
#endif
#ifndef GEN_EXE_HASH
#define GEN_EXE_HASH 0
#endif

CPU *dl_R; u8 *dl_M; u16 *dl_LOADSEG;
int *dl_tail_pending; u16 *dl_tail_sp;
int *dl_unw_active; u16 *dl_unw_sp, *dl_unw_ip, *dl_unw_cs;
int *dl_hook_bypass; volatile int *dl_poll_flag;

static const DlRuntime *rt;
void dispatch_far(u16 seg, u16 off) { rt->dispatch_far(seg, off); }
void dispatch_near(u16 cs, u16 off) { rt->dispatch_near(cs, off); }
void dispatch_jmp_far(u16 seg, u16 off) { rt->dispatch_jmp_far(seg, off); }
void dispatch_jmp_near(u16 cs, u16 off) { rt->dispatch_jmp_near(cs, off); }
void dl_poll(void) { rt->dl_poll(); }
void fatal(const char *fmt, ...)
{
    char b[600]; va_list ap;
    va_start(ap, fmt); vsnprintf(b, sizeof b, fmt, ap); va_end(ap);
    rt->fatal("%s", b);
}
u8 hle_in8(u16 port) { return rt->hle_in8(port); }
void hle_out8(u16 port, u8 v) { rt->hle_out8(port, v); }
void hle_int(int n) { rt->hle_int(n); }
void hook_run(int idx) { rt->hook_run(idx); }
void interp_run(u16 seg, u16 off) { rt->interp_run(seg, off); }
void interp_step(u16 cs, u16 ip) { rt->interp_step(cs, ip); }
void ovl_enter(int section) { rt->ovl_enter(section); }
void ret_mismatch(u16 my_sp, int kind, u16 n, u16 cs) { rt->ret_mismatch(my_sp, kind, n, cs); }

extern const unsigned INTERP_ALWAYS_TABLE[]; extern const int INTERP_ALWAYS_COUNT;
extern const DriverImage *DRIVERS[]; extern u16 *DRIVER_SEGS[]; extern u16 drvseg_mgraphic;

DLEXPORT const DlGame *dl_game_init(const DlRuntime *r)
{
    static DlGame g;
    if (!r || r->version != DL_SPLIT_VERSION) return 0;
    rt = r;
    dl_R = (CPU *)r->cpu_p; dl_M = r->mem_p; dl_LOADSEG = r->loadseg_p;
    dl_tail_pending = r->tail_pending_p; dl_tail_sp = r->tail_sp_p;
    dl_unw_active = r->unw_active_p; dl_unw_sp = r->unw_sp_p; dl_unw_ip = r->unw_ip_p; dl_unw_cs = r->unw_cs_p;
    dl_hook_bypass = r->hook_bypass_p; dl_poll_flag = r->poll_flag_p;
    g.version = DL_SPLIT_VERSION;
    g.exe_hash = GEN_EXE_HASH;
    g.func_table = FUNC_TABLE; g.func_count = &FUNC_COUNT;
    g.vec_table = VEC_TABLE; g.vec_count = &VEC_COUNT;
    g.ret_table = RET_TABLE; g.ret_count = &RET_COUNT;
    g.hook_points = HOOK_POINTS; g.hook_count = &HOOK_COUNT;
    g.hook_table = hook_table;
    g.interp_always_table = INTERP_ALWAYS_TABLE; g.interp_always_count = &INTERP_ALWAYS_COUNT;
    g.drivers = (const void *const *)DRIVERS; g.driver_segs = DRIVER_SEGS;
    g.drvseg_mgraphic = &drvseg_mgraphic;
    g.start_c = F_root_06a2a; g.start_rtlink = F_root_0a2fd;
    return &g;
}
