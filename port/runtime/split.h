/* split.h - the game's translated code as a module of its own.
 *
 * Normally the generated C (build/port_gen) is linked into the program. A release cannot ship
 * it (it is made from the player's DARKLAND.EXE), so the program can also load it at run time
 * as a module that the player's machine makes once (TinyCC, port/tools/prepare.py):
 *
 *   DL_GENSPLIT  compiling the generated code (and cpu.c, overrides.c, gensplit.c) into that
 *                module: the runtime's variables are reached through pointers it hands over,
 *                its functions through the stubs in gensplit.c;
 *   DL_RTSPLIT   compiling the program without the generated code: the generated tables and
 *                entry points come from the module (dl_game, set by gameload.c).
 *
 * Neither defined: the ordinary single program. */
#ifndef DL_SPLIT_H
#define DL_SPLIT_H

#define DL_SPLIT_VERSION 1

#ifdef DL_GENSPLIT
/* every "extern T x;" of these in the headers becomes "extern T (*dl_x);" */
#define R            (*dl_R)
#define M            (dl_M)
#define LOADSEG      (*dl_LOADSEG)
#define tail_pending (*dl_tail_pending)
#define tail_sp      (*dl_tail_sp)
#define unw_active   (*dl_unw_active)
#define unw_sp       (*dl_unw_sp)
#define unw_ip       (*dl_unw_ip)
#define unw_cs       (*dl_unw_cs)
#define hook_bypass  (*dl_hook_bypass)
#define poll_flag    (*dl_poll_flag)
#endif

/* what the program hands the module */
struct DlCpu;
typedef struct DlRuntime {
    int version;
    void *cpu_p; unsigned char *mem_p; unsigned short *loadseg_p;     /* (names apart from the macros above) */
    int *tail_pending_p; unsigned short *tail_sp_p;
    int *unw_active_p; unsigned short *unw_sp_p, *unw_ip_p, *unw_cs_p;
    int *hook_bypass_p; volatile int *poll_flag_p;
    void (*dispatch_far)(unsigned short, unsigned short);
    void (*dispatch_near)(unsigned short, unsigned short);
    void (*dispatch_jmp_far)(unsigned short, unsigned short);
    void (*dispatch_jmp_near)(unsigned short, unsigned short);
    void (*dl_poll)(void);
    void (*fatal)(const char *fmt, ...);
    unsigned char (*hle_in8)(unsigned short);
    void (*hle_out8)(unsigned short, unsigned char);
    void (*hle_int)(int);
    void (*hook_run)(int);
    void (*interp_run)(unsigned short, unsigned short);
    void (*interp_step)(unsigned short, unsigned short);
    void (*ovl_enter)(int);
    void (*ret_mismatch)(unsigned short, int, unsigned short, unsigned short);
} DlRuntime;

/* what the module hands back */
typedef struct DlGame {
    int version;
    const char *translator;                /* id of the translator that made it (prepare.py --id) */
    const void *func_table; const int *func_count;
    const void *vec_table; const int *vec_count;
    const void *ret_table; const int *ret_count;
    const void *hook_points; const int *hook_count;
    void (**hook_table)(void);
    const unsigned *interp_always_table; const int *interp_always_count;
    const void *const *drivers; unsigned short **driver_segs;
    unsigned short *drvseg_mgraphic;
    void (*start_c)(void);                 /* root 06a2a, __astart */
    void (*start_rtlink)(void);            /* root 0a2fd */
} DlGame;

#ifdef DL_RTSPLIT
extern const DlGame *dl_game;
#define FUNC_TABLE          ((const FuncEntry *)dl_game->func_table)
#define FUNC_COUNT          (*dl_game->func_count)
#define VEC_TABLE           ((const VecEntry *)dl_game->vec_table)
#define VEC_COUNT           (*dl_game->vec_count)
#define RET_TABLE           ((const RetSite *)dl_game->ret_table)
#define RET_COUNT           (*dl_game->ret_count)
#define HOOK_POINTS         ((const HookPoint *)dl_game->hook_points)
#define HOOK_COUNT          (*dl_game->hook_count)
#define hook_table          (dl_game->hook_table)
#define INTERP_ALWAYS_TABLE (dl_game->interp_always_table)
#define INTERP_ALWAYS_COUNT (*dl_game->interp_always_count)
#define DRIVERS             ((const DriverImage *const *)dl_game->drivers)
#define DRIVER_SEGS         (dl_game->driver_segs)
#define drvseg_mgraphic     (*dl_game->drvseg_mgraphic)
#endif

#endif
