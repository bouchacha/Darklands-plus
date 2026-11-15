/* test_duplicate_load.c - synthetic checks for repeated roster IDs during loading.
 * gcc -ffunction-sections -fdata-sections -Wl,--gc-sections -o build/test_duplicate_load port/tools/test_duplicate_load.c
 * build/test_duplicate_load
 * Sanitizers: add -fsanitize=address,undefined --param asan-globals=0 so unused fix registries can be discarded.
 * The full game test verifies the actual reader and chooser call sites separately.
 */
#include <assert.h>
#include "../mods/fixes.c"

u8 *M;
CPU R;
u16 LOADSEG;
int unw_active;
static int io, fail, original_mode, calls;
enum { FREAD, DOS_READ, OTHER };

int config_get_int(const char *key, int fallback)
{
    return !strcmp(key, "original") ? original_mode : fallback;
}
void dos_host_path_read(const char *name, char *out, size_t n)
{
    (void)name;
    snprintf(out, n, "/file-not-used-by-original-mode-test");
}
void hook_return_far(unsigned count) { SP = (u16)(SP + 4 + count); }
void hook_call_original(void)
{
    calls++;
    if (io == FREAD) AX = fail ? 0 : 1;
    else if (io == DOS_READ) {
        ww(DS, rw(SS, SP + 12), fail ? 3 : 10);
        AX = fail ? 5 : 0;
    }
    hook_return_far(0);
}

static void fixture(DuplicateLoad *load, const int ids[5])
{
    memset(M, 0, MEM_SIZE); memset(&R, 0, sizeof R);
    memset(load, 0, sizeof *load);
    for (int slot = 0; slot < 5; slot++) {
        load->original[2 * slot] = (u8)ids[slot];
        load->original[2 * slot + 1] = (u8)((unsigned)ids[slot] >> 8);
        memset(M + LIN(G_DGROUP, CB_REC(slot)), 0x20 + slot, 0x80);
        g_wb(G_MEMBER(slot), 1);
        memset(M + LIN(G_FAR2E38, G_ITEM(slot, 0)), 0x40 + slot, 0x180);
        memset(M + LIN(G_FAR2E38, 0x082c + slot * 20), 0x50 + slot, 20);
        memset(M + LIN(G_FAR2E38, 0x07be + slot * 22), 0x60 + slot, 22);
    }
    duplicate_load = load;
    fail = unw_active = original_mode = calls = 0;
}

static void read_ids(DuplicateLoad *load, int kind, int correct_site)
{
    io = kind;
    SP = 0xf000; DS = SS = G_DGROUP;
    memcpy(M + LIN(G_DGROUP, 0xf100), load->original, 10);
    if (kind == FREAD) {
        ww(SS, SP, correct_site ? 0x14c0 : 0x2000);
        ww(SS, SP + 2, LOADSEG + 0x1c7d);
        ww(SS, SP + 4, 0xf100); ww(SS, SP + 6, 10); ww(SS, SP + 8, 1);
        duplicate_fread_hook();
    } else {
        ww(SS, SP, correct_site ? 0x4250 : 0x2000);
        ww(SS, SP + 2, LOADSEG + 0x1462);
        ww(SS, SP + 4, 7); ww(SS, SP + 6, 0xf100); ww(SS, SP + 8, G_DGROUP);
        ww(SS, SP + 10, 10); ww(SS, SP + 12, 0xf110);
        duplicate_dos_read_hook();
    }
    assert(SP == 0xf004);
}

int main(void)
{
    M = calloc(1, MEM_SIZE); assert(M);
    DuplicateLoad load;
    for (int first = 0; first < 4; first++) {
        for (int repeated = first + 1; repeated < 5; repeated++) {
            int ids[5] = { 5, 6, 7, 8, 9 };
            ids[repeated] = ids[first];
            fixture(&load, ids);
            assert(duplicate_ids(&load, 12) == (1u << repeated));
            assert(load.first[repeated] == first);
            read_ids(&load, FREAD, 1);
            assert(load.main_read && !memcmp(M + LIN(G_DGROUP, 0xf100), load.normalized, 10));
            read_ids(&load, DOS_READ, 1);
            assert(load.roster_read && !memcmp(M + LIN(G_DGROUP, 0xf100), load.normalized, 10));
            g_wb(G_LEADER, repeated);
            duplicate_slots_clear(&load);
            assert(g_rb(G_LEADER) == first && !g_rb(G_MEMBER(repeated)));
            assert(g_rb(G_MEMBER(first)) == 1);
            assert(g_rb((u16)(CB_REC(first) + 0x3e)) == 0x20 + first);
            for (int byte = 0; byte < 0x180; byte++) assert(!rb(G_FAR2E38, G_ITEM(repeated, 0) + byte));
            assert(g_rw(G_PARTY_SIZE) == (repeated == 4 ? 4 : 5));
        }
    }
    const int all_same[5] = { 7, 7, 7, 7, 7 };
    fixture(&load, all_same); assert(duplicate_ids(&load, 12) == 30);
    for (int slot = 1; slot < 5; slot++) assert(load.first[slot] == 0);
    duplicate_slots_clear(&load); assert(g_rw(G_PARTY_SIZE) == 1);
    const int empty_or_invalid[5] = { -1, -1, -2, 12, 12 };
    fixture(&load, empty_or_invalid); assert(!duplicate_ids(&load, 12));
    assert(!memcmp(load.original, load.normalized, 10));

    fixture(&load, all_same); duplicate_ids(&load, 12);
    read_ids(&load, FREAD, 0); read_ids(&load, DOS_READ, 0);
    assert(!load.main_read && !load.roster_read);
    fail = 1;
    read_ids(&load, FREAD, 1); read_ids(&load, DOS_READ, 1);
    assert(!load.main_read && !load.roster_read);
    assert(!memcmp(M + LIN(G_DGROUP, 0xf100), load.original, 10));

    duplicate_load = NULL; io = OTHER; original_mode = 1; calls = 0;
    SP = 0xf000; duplicate_load_hook();
    assert(calls == 1 && !duplicate_load && SP == 0xf004);
    free(M);
    puts("duplicate party load checks: PASS");
    return 0;
}
