/* hooks.h - replacing or wrapping recompiled game functions from native code (mods, new UI).
 *
 *   static void my_hook(void) { hook_call_original(); AX = AX * 2; }
 *   hook_install(14, 0x18bf, 0x08ca, my_hook);      // section 14 (s014), seg:off link address
 *
 * A hook runs instead of the function, with the machine state at the function's entry (the
 * return address on the emulated stack, arguments above it). hook_call_original() runs the
 * original function there (it performs the original ret). A hook that does not call the
 * original must do the return itself: hook_return_far(n) / hook_return_near(n). */
#ifndef DL_HOOKS_H
#define DL_HOOKS_H
typedef struct { unsigned lin; int sect; int idx; } HookPoint;
#ifndef DL_RTSPLIT                       /* else from the game module (split.h) */
extern const HookPoint HOOK_POINTS[];
extern const int HOOK_COUNT;
extern void (*hook_table[])(void);
#endif
extern int hook_bypass, hook_current;
/* in generated code: run the hook unless it is calling the original */
#define HOOK(i) do { if (hook_table[i]) { if (hook_bypass == (i)) hook_bypass = -1; else { hook_run(i); return; } } } while (0)
void hook_run(int idx);
int hook_install(int sect, unsigned seg, unsigned off, void (*fn)(void));   /* sect -1 = root; 0 if no such function */
void hook_remove(int sect, unsigned seg, unsigned off);
void hook_call_original(void);
void hook_return_far(unsigned n);
void hook_return_near(unsigned n);
#endif
