/* The lookout in $McGuf02, through its card hook.
 * gcc -ffunction-sections -fdata-sections -Wl,--gc-sections -o /tmp/test_cave_watch port/tools/test_cave_watch.c && /tmp/test_cave_watch
 */
#include <assert.h>
#include "../mods/fixes.c"

u8 *M;
CPU R;
u16 LOADSEG;
static int original, watching, shown;
static void (*on_text)(void);

int config_get_int(const char *key, int fallback)
{
    return !strcmp(key, "original") ? original : fallback;
}
int hook_install(int sect, unsigned seg, unsigned off, void (*fn)(void))
{
    assert(!on_text && sect == 43 && seg == 0x150b && off == 0x03b0);
    on_text = fn;
    return 1;
}
void hook_remove(int sect, unsigned seg, unsigned off, void (*fn)(void))
{
    assert(on_text == fn);
    on_text = NULL;
}
void hook_call_original(void)
{
    if (watching) {
        shown = rw(SS, (u16)(SP + 4));
        return;
    }
    /* The watch chooses text 13 for guardian 2, which the cave never assigns. */
    watching = 1;
    ww(SS, (u16)(SP + 4), g_rw(GV_saved_card) == 2 ? 13 : 12);
    if (on_text) on_text(); else hook_call_original();
    watching = 0;
}
int main(void)
{
    M = calloc(1, MEM_SIZE);
    assert(M);
    for (original = 0; original <= 1; original++) {
        for (int guardian = 0; guardian <= 3; guardian++) {
            g_ww(GV_saved_card, (u16)guardian);
            shown = -1;
            cave_watch_hook();
            assert(shown == (guardian == 2 || (guardian == 3 && !original) ? 13 : 12));
            assert(!on_text && g_rw(GV_saved_card) == guardian);
        }
    }
    free(M);
    return 0;
}
