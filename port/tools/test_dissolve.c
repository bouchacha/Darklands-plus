/* Dissolve Lock regression checks using synthetic inventories and the real port hooks.
 * The original action's game calls are stubbed. A copied-game input test must also check
 * the hooks and item consumption in the Windows build.
 * gcc -ffunction-sections -fdata-sections -Wl,--gc-sections -o /tmp/test_dissolve port/tools/test_dissolve.c && /tmp/test_dissolve
 */
#include <assert.h>
#include <stdarg.h>
#include "../mods/fixes.c"

u8 *M;
CPU R;
u16 LOADSEG;
int unw_active, ns_unwinding;
static int original_mode, original_calls, queued;
static int charged_member, charged_slot, opened, canceled, notices, interrupt_message;

int config_get_int(const char *key, int def)
{
    return !strcmp(key, "original") ? original_mode : def;
}
void hook_return_far(unsigned n) { SP = (u16)(SP + 4 + n); }
void hook_return_near(unsigned n) { SP = (u16)(SP + 2 + n); }
void ui_notice(const char *message) { assert(strstr(message, "Eater-Water")); notices++; }

static void consume(int member, int slot)
{
    charged_member = member; charged_slot = slot;
    assert(member >= 0 && member < 5 && slot >= 0 && slot < 64);
    u16 item = G_ITEM(member, slot);
    unsigned count = rb(G_FAR2E38, (u16)(item + 4));
    assert(count > 0);
    wb(G_FAR2E38, (u16)(item + 4), (u8)(count - 1));
}

int ns_call(int sect, u16 seg, u16 off, int n, ...)
{
    int args[4] = { 0 };
    va_list ap; va_start(ap, n);
    for (int i = 0; i < n; i++) args[i] = va_arg(ap, int);
    va_end(ap);
    if (sect == 14 && seg == 0x18bf && off == 0x0fa4) consume(args[0], args[1]);
    if (sect == 23 && (off == 0x00b8 || off == 0x13d0)) {
        assert((off == 0x00b8 ? args[1] : args[0]) == (s8)g_rb(G_LEADER));
        opened++;
    }
    if (sect == 14 && off == 0x27ac) {
        canceled++;
        g_wb((u16)(GV_cbt_special + args[0]), 0);
    }
    if (sect == -1 && seg == 0 && off == 0x24bc && interrupt_message) ns_unwinding = 1;
    return 0;
}

void hook_call_original(void)
{
    original_calls++;
    if (queued) {
        int member = (s16)rw(SS, SP + 2);
        consume(member, g_rb((u16)(DISSOLVE_SLOT + member)));
        opened++;
        hook_return_near(0);
    } else hook_return_far(0);
}

static void item(int member, int slot, int code, int quality, int count)
{
    u16 at = G_ITEM(member, slot);
    ww(G_FAR2E38, at, (u16)code);
    wb(G_FAR2E38, at + 3, (u8)quality);
    wb(G_FAR2E38, at + 4, (u8)count);
}

static void reset(int leader)
{
    memset(M, 0, MEM_SIZE); memset(&R, 0, sizeof R);
    SS = DS = G_DGROUP; SP = 0x8000;
    original_mode = original_calls = queued = opened = canceled = notices = interrupt_message = 0;
    unw_active = ns_unwinding = 0; charged_member = charged_slot = -1;
    g_wb(G_LEADER, (u8)leader); g_wb(DISSOLVE_ENABLED, 1); g_wb(DISSOLVE_DOOR, 1);
    for (int member = 0; member < 5; member++) {
        g_wb(G_MEMBER(member), 1);
        g_wb((u16)(G_MEMBER(member) + G_M_NITEMS), 4);
        item(member, 0, 10, 25, 3);
        item(member, 1, 0x63, 45, 2);
        item(member, 2, 0x63, 25, 2);
        item(member, 3, 12, 25, 3);
        g_wb((u16)(DISSOLVE_SLOT + member), 0);
    }
}

int main(void)
{
    M = calloc(1, MEM_SIZE); assert(M);
    for (int leader = 0; leader < 5; leader++) {
        for (int selected = -1; selected < 5; selected++) {
            reset(leader); g_wb(GV_selected_member, (u8)selected);
            dissolve_key_hook();
            assert(SP == 0x8004 && !original_calls && opened == 1 && canceled == 1);
            assert(charged_member == leader && charged_slot == 2);
            assert(rb(G_FAR2E38, G_ITEM(leader, 2) + 4) == 1);
            assert(rb(G_FAR2E38, G_ITEM(leader, 0) + 4) == 3);
            assert((s8)g_rb(GV_selected_member) == selected);
        }
        reset(leader); queued = 1; ww(SS, SP + 2, (u16)leader);
        g_wb((u16)(GV_cbt_special + leader), 7);
        dissolve_order_hook();
        assert(SP == 0x8002 && original_calls == 1 && opened == 1);
        assert(charged_member == leader && charged_slot == 2);

        reset(leader); queued = 1; ww(SS, SP + 2, (u16)leader);
        item(leader, 1, 11, 25, 3); item(leader, 2, 12, 25, 3);
        g_wb((u16)(GV_cbt_special + leader), 7);
        dissolve_order_hook();
        assert(SP == 0x8002 && !original_calls && !opened && canceled == 1 && notices == 1);
        assert(charged_member == -1 && g_rb((u16)(DISSOLVE_SLOT + leader)) == 255);
    }
    reset(2); g_wb(DISSOLVE_DOOR, 0); dissolve_key_hook(); assert(opened == 1);
    reset(2); g_wb((u16)(G_MEMBER(2) + G_M_NITEMS), 0); dissolve_key_hook();
    assert(!opened && notices == 1 && canceled == 1 && charged_member == -1);
    reset(-1); dissolve_key_hook(); assert(!opened && notices == 1 && !canceled);
    reset(2); interrupt_message = 1; dissolve_key_hook();
    assert(ns_unwinding && !opened && !canceled && charged_member == -1 && SP == 0x8000);
    reset(2); original_mode = 1; dissolve_key_hook(); assert(original_calls == 1 && !opened);
    reset(2); g_wb(GV_ai_paused, 1); dissolve_key_hook(); assert(original_calls == 1 && !opened);
    reset(2); g_wb(DISSOLVE_ENABLED, 2); dissolve_key_hook(); assert(original_calls == 1 && !opened);
    reset(2); item(2, 2, 0x63, 25, 0); dissolve_key_hook(); assert(charged_slot == 1);
    reset(2); g_wb((u16)(DISSOLVE_SLOT + 2), 255); dissolve_key_hook(); assert(charged_slot == 2);
    reset(2); g_wb((u16)(G_MEMBER(2) + G_M_NITEMS), 64); item(2, 63, 0x63, 20, 2);
    dissolve_key_hook(); assert(charged_slot == 63);
    reset(2); g_wb(G_MEMBER(2), 2); dissolve_key_hook(); assert(!opened && charged_member == -1);
    reset(2); original_mode = 1; queued = 1; ww(SS, SP + 2, 2);
    g_wb((u16)(GV_cbt_special + 2), 7); dissolve_order_hook();
    assert(SP == 0x8002 && original_calls == 1 && charged_slot == 0);
    free(M);
    puts("dissolve inventory regression checks: PASS");
    return 0;
}
