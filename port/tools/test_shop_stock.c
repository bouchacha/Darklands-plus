/* The original stock routine reads [bp-2] after pushing BP. Exercise the
 * native hook with the values the uninitialized word gets during a listing.
 * gcc -ffunction-sections -fdata-sections -Wl,--gc-sections -o /tmp/test_shop_stock port/tools/test_shop_stock.c && /tmp/test_shop_stock
 */
#include <assert.h>
#include "../mods/fixes.c"

u8 *M;
CPU R;
u16 LOADSEG;
static int original, calls;
static u16 stock_input;
int config_get_int(const char *key, int fallback)
{
    return !strcmp(key, "original") ? original : fallback;
}
void hook_call_original(void)
{
    calls++;
    stock_input = rw(SS, (u16)(SP - 4));
}
int main(void)
{
    static const u16 places[] = { 0, 31, 91, 92, 376, 0xffff };
    static const u16 old_words[] = { 0, 1, 8, 0x1000, 0xffff };
    M = calloc(1, MEM_SIZE); assert(M);
    SP = 0x8000;
    for (original = 0; original <= 1; original++) {
        for (unsigned p = 0; p < sizeof places / sizeof places[0]; p++) {
            g_ww(G_CUR_LOC, places[p]);
            for (unsigned w = 0; w < sizeof old_words / sizeof old_words[0]; w++) {
                ww(SS, SP - 4, old_words[w]);
                calls = 0;
                shopstock_hook();
                assert(calls == 1);
                assert(stock_input == (places[p] < 92 && !original ? places[p] : old_words[w]));
            }
        }
    }
    free(M);
    return 0;
}
