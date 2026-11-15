/* Inn roster regression checks using synthetic character headers and the real port hooks.
 * gcc -ffunction-sections -fdata-sections -Wl,--gc-sections -o /tmp/test_inn_roster port/tools/test_inn_roster.c && /tmp/test_inn_roster
 * The window and picture calls are stubbed. Native and original screen input is checked
 * separately in a copied game folder. No game data is needed by this test. */
#include <assert.h>
#include <stdarg.h>
#include <unistd.h>
#include "../mods/fixes.c"

u8 *M;
CPU R;
u16 LOADSEG;
int unw_active, ns_unwinding;
static char roster_path[] = "/tmp/dl-inn-roster-XXXXXX";
static int listing, original_calls, original_row, last_picture, expected_record;
static int list_entries[20], listed;
static char picture_name[64];

void dos_host_path_read(const char *name, char *out, size_t n)
{
    assert(!strcmp(name, "charactr.tmp"));
    snprintf(out, n, "%s", roster_path);
}

void hook_return_far(unsigned n) { SP = (u16)(SP + 4 + n); }
void hook_call_original(void)
{
    original_calls++;
    if (listing) {
        for (int row = 0; row < listed; row++) g_ww((u16)(INN_ROWS + 2 * row), (u16)list_entries[row]);
        g_ww(GV_roster_n, 30);             /* the original counts every record, including those elsewhere */
        SP += 2;                         /* the list is a near routine */
    } else {
        original_row = (s16)g_rw(GV_roster_sel);
        assert(original_row == expected_record);
        AX = 0x7654; DX = 0x4321;
        hook_return_far(0);
    }
}

u16 ns_stack_str(const char *s)
{
    SP -= (u16)(strlen(s) + 2);
    for (unsigned i = 0; i <= strlen(s); i++) wb(SS, (u16)(SP + i), (u8)s[i]);
    return SP;
}
void ns_stack_free(u16 sp) { SP = sp; }
int ns_call(int sect, u16 seg, u16 off, int n, ...)
{
    int args[12];
    va_list ap; va_start(ap, n);
    for (int k = 0; k < n; k++) args[k] = va_arg(ap, int);
    va_end(ap);
    if (sect == -1 && seg == 0x0617 && off == 0x045c) {
        unsigned k = 0;
        do { picture_name[k] = (char)rb(SS, (u16)(args[0] + k)); } while (picture_name[k++] && k < sizeof picture_name);
        return 0x5000;
    }
    if (sect == -1 && seg == 0x047c && off == 0x05a5) last_picture = args[3];
    return 0;
}

static void fixture(void)
{
    memset(M, 0, MEM_SIZE); memset(&R, 0, sizeof R);
    SS = DS = G_DGROUP; SP = 0x8000;
    original_calls = 0; original_row = -1; last_picture = 0; picture_name[0] = 0;
    listing = 0; ns_unwinding = 0;
    u8 header[32] = { 30, 0, 7, 0, 21, 0, 9, 0, 0xff, 0xff, 0xff, 0xff };
    memcpy(header + 12, "F01\0F60\0C00\0", 12);
    FILE *file = fopen(roster_path, "wb"); assert(file);
    assert(fwrite(header, 1, sizeof header, file) == sizeof header); fclose(file);
    for (int member = 0; member < 3; member++) {
        g_wb(G_MEMBER(member), 1);
        for (int k = 0; k < 24; k++) wb(G_FAR20A5, (u16)(0x33fc + member * 24 + k), (u8)(member * 20 + k));
    }
    g_ww(GV_roster_n, 5);
    const int rows[] = { 0, 7, 5, 9, 21 };
    for (int row = 0; row < 5; row++) g_ww((u16)(INN_ROWS + 2 * row), (u16)rows[row]);
}

static void press(int key)
{
    SP = 0x8000; ww(SS, SP + 4, (u16)key);
    innroster_key_hook();
    assert(SP == 0x8004);
}

int main(void)
{
    int fd = mkstemp(roster_path); assert(fd >= 0); close(fd);
    M = calloc(1, MEM_SIZE); assert(M);

    fixture(); g_ww(GV_roster_sel, 3);
    press(0x5000);                         /* fifth visible row names roster record 21 */
    assert(g_rw(GV_roster_sel) == 4 && !original_calls);
    assert(!strcmp(picture_name, "pics\\F60small.pic") && last_picture == 0x5000);
    for (int k = 0; k < 24; k++) assert(g_rb(INN_PORTRAIT_COLOURS + k) == 20 + k);
    press(0x5000);                         /* wrap to an inactive character, with no portrait */
    assert(g_rw(GV_roster_sel) == 0 && !original_calls);
    press(0x4800);
    assert(g_rw(GV_roster_sel) == 4 && !strcmp(picture_name, "pics\\F60small.pic"));

    const int keys[] = { 0x231, 0x4f31, 0x332, 0x5032, 0x433, 0x5133 };
    for (unsigned k = 0; k < sizeof keys / sizeof keys[0]; k++) {
        fixture(); g_ww(GV_roster_sel, 4); expected_record = 21;
        press(keys[k]);
        assert(original_row == 21 && original_calls == 1 && g_rw(GV_roster_sel) == 4);
        assert(AX == 0x7654 && DX == 0x4321);
    }
    fixture(); g_ww(GV_roster_sel, 1); expected_record = 1;
    press(0x2348);                          /* H already resolves rows in the original */
    assert(original_row == 1 && g_rw(GV_roster_sel) == 1);

    const int bad_rows[] = { -1, 5, 20, 32767 };
    for (unsigned k = 0; k < sizeof bad_rows / sizeof bad_rows[0]; k++) {
        fixture(); g_ww(GV_roster_sel, (u16)bad_rows[k]);
        press(0x231); press(0x2348); press(0x1f53);
        assert(!original_calls);
    }
    fixture(); g_ww(GV_roster_sel, 0);       /* a visible character outside the party */
    press(0x231); assert(!original_calls);
    fixture(); g_ww(GV_roster_n, 0); press(0x5000);
    assert((s16)g_rw(GV_roster_sel) == -1 && !original_calls);

    fixture(); listing = 1; listed = 2; list_entries[0] = 7; list_entries[1] = 21;
    g_ww(GV_roster_sel, 4); innroster_list_hook();
    assert(SP == 0x8002 && g_rw(GV_roster_n) == 2 && g_rw(GV_roster_sel) == 0);
    assert((s16)g_rw(INN_ROWS + 4) == -1);  /* an old third row cannot select an earlier occupant */
    listed = 0; SP = 0x8000; innroster_list_hook();
    assert(g_rw(GV_roster_n) == 0 && (s16)g_rw(GV_roster_sel) == -1);
    listing = 0; expected_record = -1;
    press(0x1352);                         /* Return still reaches the screen's normal exit with no rows */
    assert(original_row == -1);

    unlink(roster_path); free(M);
    puts("inn roster regression checks: PASS");
    return 0;
}
