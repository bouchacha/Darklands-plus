/* test_leaving_record.c - synthetic checks of a departing companion's full roster write.
 * gcc -ffunction-sections -fdata-sections -Wl,--gc-sections -o build/test_leaving_record port/tools/test_leaving_record.c
 * build/test_leaving_record
 * Also exercise party_remove and party_add in a copied game to check the hook sites.
 */
#include <assert.h>
#include "../mods/fixes.c"

u8 *M;
CPU R;
u16 LOADSEG;
int unw_active;
static int original_mode, member_now, in_write, fail_write, abort_remove;
static unsigned written;
static u8 stored[0x22a], text_before[0x22a];

int config_get_int(const char *key, int fallback)
{
    return !strcmp(key, "original") ? original_mode : fallback;
}
void hook_return_far(unsigned count) { SP = (u16)(SP + 4 + count); }

static void enter_write(int member, unsigned count)
{
    in_write = 1;
    SS = DS = G_DGROUP;
    SP = 0xf000;
    ww(SS, SP + 4, 7);
    ww(SS, SP + 6, CB_REC(member));
    ww(SS, SP + 8, G_DGROUP);
    ww(SS, SP + 10, count);
    ww(SS, SP + 12, 0xf080);
}

void hook_call_original(void)
{
    if (!in_write) {
        if (abort_remove) { unw_active = 1; return; }
        if (g_rb(G_MEMBER(member_now)) != 5) g_wb(G_MEMBER(member_now), 0);
        g_ww(CB_REC(member_now), 86);
        enter_write(member_now, 0x80);
        leaving_record_write_hook();
        g_wb(G_MEMBER(member_now), 0);
        return;
    }
    unsigned count = rw(SS, SP + 10);
    if (!fail_write) {
        assert(count <= sizeof stored);
        memcpy(stored, M + LIN(rw(SS, SP + 8), rw(SS, SP + 6)), count);
        written = count;
    }
    ww(DS, rw(SS, SP + 12), (u16)written);
    AX = fail_write ? 5 : 0;
    hook_return_far(0);
}

static void reset(int member, int status)
{
    memset(M, 0, MEM_SIZE);
    memset(&R, 0, sizeof R);
    memset(stored, 0xee, sizeof stored);
    SS = DS = G_DGROUP;
    SP = 0xf100;
    ww(SS, SP + 4, (u16)member);
    in_write = fail_write = abort_remove = unw_active = original_mode = 0;
    written = 0;
    departing_member = -1;
    member_now = member;
    memset(M + LIN(G_DGROUP, CB_REC(member)), 0x30 + member, 0x80);
    g_wb(G_MEMBER(member), (u8)status);
    memset(M + LIN(G_FAR2E38, 0x082c + 20 * member), 0x40 + member, 20);
    memset(M + LIN(G_FAR2E38, 0x07be + 22 * member), 0x50 + member, 22);
    memset(M + LIN(G_FAR2E38, G_ITEM(member, 0)), 0x60 + member, 0x180);
    memset(M + LIN(G_DGROUP, GV_msg_buf), 0x7a, sizeof text_before);
    memcpy(text_before, M + LIN(G_DGROUP, GV_msg_buf), sizeof text_before);
}

static void check_member(int member, int status)
{
    reset(member, status);
    u8 expected[0x22a];
    memcpy(expected, M + LIN(G_DGROUP, CB_REC(member)), 0x80);
    expected[0] = 86; expected[1] = 0;
    expected[0x14] = status == 5 ? 5 : 0;
    memcpy(expected + 0x80, M + LIN(G_FAR2E38, 0x082c + 20 * member), 20);
    memcpy(expected + 0x94, M + LIN(G_FAR2E38, 0x07be + 22 * member), 22);
    memcpy(expected + 0xaa, M + LIN(G_FAR2E38, G_ITEM(member, 0)), 0x180);
    leaving_record_hook();
    assert(written == sizeof expected && !memcmp(stored, expected, sizeof expected));
    assert(!memcmp(text_before, M + LIN(G_DGROUP, GV_msg_buf), sizeof text_before));
    assert(!g_rb(G_MEMBER(member)) && departing_member == -1);
    assert(SP == 0xf004 && rw(SS, 0xf006) == CB_REC(member) && rw(SS, 0xf00a) == 0x80);
}

int main(void)
{
    M = calloc(1, MEM_SIZE); assert(M);
    for (int member = 0; member < 5; member++) {
        check_member(member, 1);
        check_member(member, 5);
    }
    reset(2, 1); original_mode = 1;
    leaving_record_hook();
    assert(written == 0x80 && stored[0x80] == 0xee && departing_member == -1);

    reset(2, 1); enter_write(2, 0x80);
    leaving_record_write_hook(); /* an unrelated _dos_write has no departure context */
    assert(written == 0x80 && stored[0x80] == 0xee);

    reset(2, 1); fail_write = 1;
    leaving_record_hook();
    assert(AX == 5 && !written && departing_member == -1);
    assert(!memcmp(text_before, M + LIN(G_DGROUP, GV_msg_buf), sizeof text_before));

    reset(2, 1); abort_remove = 1;
    leaving_record_hook();
    assert(unw_active && !written && departing_member == -1);
    check_member(4, 1); /* another departure after the aborted call */
    free(M);
    puts("departing member record checks: PASS");
    return 0;
}
