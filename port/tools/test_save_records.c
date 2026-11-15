/* Save writer regression checks with synthetic roster records and the real hooks.
 * The original writer's record loop is isolated here. The native test must also save
 * and reload a copied game, since the I/O hook sites are addresses in 483.07.
 * gcc -ffunction-sections -fdata-sections -Wl,--gc-sections -o /tmp/test_save_records port/tools/test_save_records.c && /tmp/test_save_records
 */
#include <assert.h>
#include "../mods/fixes.c"

u8 *M;
CPU R;
u16 LOADSEG;
int unw_active;
static int original_mode, use_fix, io, written, read_record, aborted;
static u8 roster[10][0x22a], output[10][0x22a], saved_ids[10];
static const s16 fixture_ids[5] = { 1, 3, 4, 6, 8 };
enum { WRITER, IDS_WRITE, ROSTER_READ };
enum { FRAME = 0xf000, ID_LIST = FRAME - 0x72, BUFFER = 0x9085 };

int config_get_int(const char *key, int def) { return !strcmp(key, "original") ? original_mode : def; }
void hook_return_far(unsigned n) { SP = (u16)(SP + 4 + n); }

static void enter_io(int kind, u16 return_ip)
{
    io = kind;
    BP = FRAME; SP = 0xee00; SS = DS = G_DGROUP;
    ww(SS, SP, return_ip); ww(SS, SP + 2, LOADSEG + 0x1462);
}
static void live_record(int member, u8 out[0x22a])
{
    memcpy(out, M + LIN(G_DGROUP, CB_REC(member)), 0x80);
    memcpy(out + 0x80, M + LIN(G_FAR2E38, 0x082c + 20 * member), 20);
    memcpy(out + 0x94, M + LIN(G_FAR2E38, 0x07be + 22 * member), 22);
    memcpy(out + 0xaa, M + LIN(G_FAR2E38, G_ITEM(member, 0)), 0x180);
}

static void original_writer(void)
{
    if (aborted == 3) return;
    for (int slot = 0; slot < 5; slot++) ww(G_DGROUP, ID_LIST + 2 * slot, fixture_ids[slot]);
    enter_io(IDS_WRITE, 0x34ee);
    ww(SS, SP + 4, ID_LIST); ww(SS, SP + 6, 10); ww(SS, SP + 8, 1);
    if (use_fix) save_records_header_hook(); else hook_call_original();
    if (aborted) { unw_active = aborted == 2; return; }
    for (int record = 0; record < 10; record++) {
        int member;
        for (member = 0; member < 5; member++) if ((s16)g_rw(ID_LIST + 2 * member) == record) break;
        if (member < 5) {
            if (g_rb(G_MEMBER(member))) {
                live_record(member, output[written++]);
                read_record++;
            }
        } else {
            enter_io(ROSTER_READ, 0x361d);
            ww(SS, FRAME - 0x5c, record);
            ww(SS, SP + 4, 7); ww(SS, SP + 6, BUFFER); ww(SS, SP + 8, G_DGROUP);
            ww(SS, SP + 10, 0x22a);
            if (use_fix) save_records_read_hook(); else hook_call_original();
            memcpy(output[written++], M + LIN(G_DGROUP, BUFFER), 0x22a);
        }
    }
}

void hook_call_original(void)
{
    if (io == WRITER) { original_writer(); return; }
    if (io == IDS_WRITE) {
        memcpy(saved_ids, M + LIN(G_DGROUP, ID_LIST), 10);
        AX = aborted == 1 ? 0 : 1;       /* short/failed fwrite does not enable replacement */
    } else {
        memcpy(M + LIN(G_DGROUP, BUFFER), roster[read_record++], 0x22a);
        AX = 0;
    }
    hook_return_far(0);
}

static void reset(unsigned absent)
{
    memset(M, 0, MEM_SIZE); memset(&R, 0, sizeof R);
    unw_active = 0; written = read_record = aborted = original_mode = 0;
    saving_records = 0; use_fix = 1; io = WRITER;
    for (int record = 0; record < 10; record++) memset(roster[record], record + 40, sizeof roster[record]);
    for (int slot = 0; slot < 5; slot++) {
        memset(M + LIN(G_DGROUP, CB_REC(slot)), slot + 10, 0x80);
        g_wb(G_MEMBER(slot), (absent & (1u << slot)) ? 0 : 1);
        memset(M + LIN(G_FAR2E38, 0x082c + 20 * slot), slot + 20, 20);
        memset(M + LIN(G_FAR2E38, 0x07be + 22 * slot), slot + 25, 22);
        memset(M + LIN(G_FAR2E38, G_ITEM(slot, 0)), slot + 30, 0x180);
    }
}
static void check_fixed(unsigned absent)
{
    reset(absent);
    u8 before[5][0x22a];
    for (int member = 0; member < 5; member++) live_record(member, before[member]);
    save_records_hook();
    assert(!saving_records && written == 10 && read_record == 10);
    for (int member = 0; member < 5; member++) {
        assert(saved_ids[2 * member] == fixture_ids[member] && !saved_ids[2 * member + 1]);
        assert(!memcmp(output[fixture_ids[member]], before[member], 0x22a));
        u8 after[0x22a]; live_record(member, after);
        assert(!memcmp(before[member], after, sizeof after));
    }
    for (int record = 0; record < 10; record++) {
        int member;
        for (member = 0; member < 5; member++) if (fixture_ids[member] == record) break;
        if (member == 5) assert(!memcmp(output[record], roster[record], 0x22a));
    }
}

int main(void)
{
    M = calloc(1, MEM_SIZE); assert(M);
    for (unsigned mask = 0; mask < 32; mask++) check_fixed(mask);
    reset(2); use_fix = 0; original_writer();
    assert(written == 9 && read_record == 9);       /* the original omits a record and shifts the next inactive one */
    assert(memcmp(output[4], roster[5], 0x22a));
    reset(2); original_mode = 1; save_records_hook();
    assert(written == 9 && !saving_records);
    for (int fail = 1; fail <= 3; fail++) {
        reset(2); aborted = fail; save_records_hook();
        assert(!saving_records);
        unw_active = 0;
        enter_io(ROSTER_READ, 0x361d);
        ww(SS, SP + 6, BUFFER); ww(SS, SP + 8, G_DGROUP); ww(SS, SP + 10, 0x22a);
        save_records_read_hook();
        assert(!memcmp(M + LIN(G_DGROUP, BUFFER), roster[0], 0x22a));
    }
    check_fixed(16);                    /* another save after a canceled or failed attempt */
    free(M);
    puts("save record regression checks: PASS");
    return 0;
}
