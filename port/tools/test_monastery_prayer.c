/* The clerk's paid prayer: exercise the scoped text and Divine Favor hooks with a
 * synthetic party. release_check.py runs this through run_tests.py.
 * gcc -ffunction-sections -fdata-sections -Wl,--gc-sections -o /tmp/test_monastery_prayer port/tools/test_monastery_prayer.c
 */
#include <assert.h>
#include <stdarg.h>
#include "../mods/fixes.c"

u8 *M;
CPU R;
u16 LOADSEG;
int ns_unwinding;
static int original_mode, scoped, original_calls, returned, reward[5];
static void (*text_hook)(void), (*favor_hook)(void);

int config_get_int(const char *key, int def)
{
    return !strcmp(key, "original") ? original_mode : def;
}
int hook_install(int section, unsigned segment, unsigned offset, void (*fn)(void))
{
    if (fn == monastery_prayer_text) text_hook = fn;
    else { assert(fn == monastery_prayer_favor); favor_hook = fn; }
    return 1;
}
void hook_remove(int section, unsigned segment, unsigned offset, void (*fn)(void))
{
    if (fn == text_hook) text_hook = NULL;
    else { assert(fn == favor_hook); favor_hook = NULL; }
}
void hook_call_original(void)
{
    original_calls++;
    if (scoped >= 0) assert(!!text_hook == scoped && !!favor_hook == scoped);
}
void hook_return_far(unsigned n) { assert(!n); returned++; }
int ns_call(int section, u16 segment, u16 offset, int nargs, ...)
{
    va_list args;
    va_start(args, nargs);
    int member = va_arg(args, int), stat = va_arg(args, int);
    assert(section == 29 && segment == 0x0e76 && member >= 0 && member < 5);
    int result = 0;
    if (offset == 0x01a0) {
        assert(stat == 8 && nargs == 2);
        result = g_rb((u16)(G_MEMBER(member) + G_M_SKILLS + stat));
    } else {
        assert(offset == 0x0a72 && stat == 6 && nargs == 3);
        reward[member] += va_arg(args, int);
    }
    va_end(args);
    return result;
}

int main(void)
{
    M = calloc(1, MEM_SIZE); assert(M); SS = G_DGROUP; SP = 0x8000;
    for (original_mode = 0; original_mode < 2; original_mode++) {
        for (int digit = 0; digit < 10; digit++) {
            scoped = !original_mode && digit >= 5 && digit <= 8;
            g_ww(G_CUR_LOC, (u16)(200 + digit));
            monastery_prayer_hook();
            assert(!text_hook && !favor_hook);
        }
    }
    scoped = -1; original_calls = 0;
    ww(SS, SP + 2, LOADSEG + 0x1a12);
    ww(SS, SP, 0x068b); ww(SS, SP + 4, 5); monastery_prayer_text();
    assert(rw(SS, SP + 4) == 4);
    ww(SS, SP, 0x05fb); ww(SS, SP + 4, 3); monastery_prayer_text();
    assert(rw(SS, SP + 4) == 3 && original_calls == 2);

    g_wb(G_MEMBER(0), 1); g_wb(G_MEMBER(2), 1); g_wb(G_MEMBER(4), 1);
    g_wb(G_MEMBER(0) + G_M_SKILLS + 8, 30); g_wb(G_MEMBER(4) + G_M_SKILLS + 8, 99);
    ww(SS, SP, 0x0699); monastery_prayer_favor();
    assert(returned == 1 && original_calls == 2);
    assert(reward[0] == 7 && reward[1] == 0 && reward[2] == 2 && reward[3] == 0 && reward[4] == 18);
    ww(SS, SP, 0x0625); monastery_prayer_favor();
    assert(original_calls == 3 && returned == 1);
    free(M);
    return 0;
}
