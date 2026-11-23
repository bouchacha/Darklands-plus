/* test_cachefile.c - synthetic cache files, atomic replacement and transfer rollback.
 * No game assets are required. From the repository root:
 * gcc -Wall -Wextra -Wno-unused-function -ffunction-sections -fdata-sections \
 *   -Wl,--gc-sections -o build/test_cachefile port/tools/test_cachefile.c && build/test_cachefile
 */
#include <assert.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../mods/cachefile.c"

u8 *M;
CPU R;
u16 LOADSEG;
int ns_unwinding, unw_active;
static int enabled = 1, original_mode, original_calls, redraws, notices;
static char folder[] = "/tmp/dl-cachefile-XXXXXX";

int fix_is_on(const char *id) { assert(!strcmp(id, "cachefile")); return enabled && !original_mode; }
void dos_host_path(const char *name, char *out, size_t n)
{
    assert(!strcmp(name, "CACHE.TMP")); snprintf(out, n, "%s/cache.tmp", folder);
}
void dos_host_path_write(const char *name, char *out, size_t n) { dos_host_path(name, out, n); }
void dos_host_path_read(const char *name, char *out, size_t n) { dos_host_path(name, out, n); }
void ui_notice(const char *message) { assert(strstr(message, "stay where")); notices++; }
void hook_return_far(unsigned n) { SP = (u16)(SP + 4 + n); }
void hook_call_original(void)
{
    original_calls++;
    /* A successful put moved the last item to a new one-entry store. */
    g_wb(G_MEMBER(0) + G_M_NITEMS, 0);
    wb(G_FAR2E38, G_ITEM(0, 0) + 4, 0);
    g_ww(GV_cache_n, 1); g_ww(GV_cache_n_shown, 1);
    wb(G_FAR2E38, CACHE_ITEMS, 56); wb(G_FAR2E38, CACHE_ITEMS + 1, 0);
    wb(G_FAR2E38, CACHE_ITEMS + 2, 25); wb(G_FAR2E38, CACHE_ITEMS + 3, 1);
    hook_return_far(0);
}
int ns_call(int section, u16 segment, u16 offset, int nargs, ...)
{
    assert(section == 35 && segment == 0x1462 && offset == 0x0782 && nargs == 2);
    redraws++; return 0;
}

static size_t fixture(u8 *data, unsigned stores, unsigned entries)
{
    unsigned header = 2 + 2 * stores;
    if (header < 198) header = 198;
    memset(data, 0, 70000); data[0] = 99; data[1] = (u8)stores;
    size_t at = header;
    for (unsigned h = 1; h <= stores; h++) {
        write_word(data + 2 * h, (unsigned)at);
        data[at++] = (u8)entries;
        for (unsigned i = 0; i < entries; i++) {
            data[at++] = (u8)(h + i); data[at++] = 0;
            data[at++] = (u8)(h + 20); data[at++] = (u8)(i + 1);
        }
    }
    return at;
}
static const u8 *record(const u8 *data, unsigned handle) { return data + read_word(data + 2 * handle); }
static void write_fixture(const u8 *data, size_t size)
{
    char path[1200]; dos_host_path("CACHE.TMP", path, sizeof path);
    FILE *file = fopen(path, "wb"); assert(file);
    assert(fwrite(data, 1, size, file) == size); assert(!fclose(file));
}
static size_t read_fixture(u8 *data)
{
    char path[1200]; dos_host_path("CACHE.TMP", path, sizeof path);
    FILE *file = fopen(path, "rb"); assert(file);
    size_t size = fread(data, 1, 70000, file); assert(!ferror(file)); fclose(file);
    return size;
}
static void state(void)
{
    memset(M, 0, MEM_SIZE); memset(&R, 0, sizeof R);
    SS = DS = G_DGROUP; SP = 0x8000; ns_unwinding = 0;
    original_calls = redraws = notices = original_mode = 0; enabled = 1;
    /* g_location(0) reads a far pointer from the location table. */
    ww(G_FAR2E38, 0x10be, 0x100); ww(G_FAR2E38, 0x10c0, 0x6100);
    g_ww(G_CUR_LOC, 0); ww(0x6100, 0x118, 0xffff);
    g_wb(G_MEMBER(0), 1); g_wb(G_MEMBER(0) + G_M_NITEMS, 1);
    wb(G_FAR2E38, G_ITEM(0, 0), 56); wb(G_FAR2E38, G_ITEM(0, 0) + 3, 25);
    wb(G_FAR2E38, G_ITEM(0, 0) + 4, 1);
}

int main(void)
{
    assert(mkdtemp(folder)); M = calloc(1, MEM_SIZE); assert(M);
    u8 *old = malloc(70000), *out = malloc(70000), *got = malloc(70000); assert(old && out && got);
    const u8 replacement[] = { 56, 0, 33, 7 };
    size_t size = fixture(old, 2, 1), result = 0;
    assert(!cache_rewrite(old, size, 1, 0, replacement, 1, out, &result));
    /* up to 49 stores, the original's own form: 99, the count, the offsets, zeros to 101, the stores in order */
    assert(result == size - 97 && out[0] == 99 && out[1] == 2 && read_word(out + 2) == 101);
    assert((signed char)out[2] == 101);                       /* (what the original's rewrite steps by) */
    assert(!memcmp(record(out, 1) + 1, replacement, 4));
    assert(!memcmp(record(out, 2), record(old, 2), 5));
    /* Reordered, gapped offsets are followed independently. */
    memcpy(old + 250, record(old, 1), 5); write_word(old + 2, 250);
    assert(!cache_rewrite(old, 255, 2, 0, replacement, 1, out, &result));
    assert(!memcmp(record(out, 1), old + 250, 5));
    /* Signed-byte counts and a high-byte first offset remain whole. */
    size = fixture(old, 100, 130);
    assert(!cache_rewrite(old, size, 2, 0, replacement, 1, out, &result));
    assert(!memcmp(record(out, 99), record(old, 99), 521));
    /* One atomic merge updates both stores, retaining the source handle empty. */
    size = fixture(old, 2, 1);
    assert(!cache_rewrite(old, size, 1, 2, replacement, 1, out, &result));
    assert(out[1] == 2 && !record(out, 2)[0]);
    assert(!memcmp(record(out, 1) + 1, replacement, 4));
    size = fixture(old, 97, 0);
    assert(!cache_rewrite(old, size, -1, 0, replacement, 1, out, &result));
    assert(out[1] == 98 && read_word(out + 2) == 198 && !record(out, 1)[0]);
    assert(!memcmp(record(out, 98) + 1, replacement, 4));
    size = fixture(old, 98, 0);
    assert(cache_rewrite(old, size, -1, 0, replacement, 1, out, &result));
    size = fixture(old, 255, 0);
    assert(cache_rewrite(old, size, -1, 0, replacement, 1, out, &result));
    assert(!cache_rewrite(old, size, 255, 0, replacement, 1, out, &result));
    size = fixture(old, 65, 250);
    assert(cache_rewrite(old, size, -1, 0, old + 199, 100, out, &result));
    assert(cache_rewrite(old, 3, 1, 0, replacement, 1, out, &result));
    /* a store that cannot be read is written back empty, the others kept (the one being written is replaced) */
    size = fixture(old, 2, 1); write_word(old + 2, 65000);
    assert(!cache_rewrite(old, size, 1, 0, replacement, 1, out, &result) && !cache_dropped);
    assert(!memcmp(record(out, 1) + 1, replacement, 4) && !memcmp(record(out, 2), record(old, 2), 5));
    size = fixture(old, 3, 1); write_word(old + 4, 65000);
    assert(!cache_rewrite(old, size, 1, 0, replacement, 1, out, &result) && cache_dropped == 1);
    assert(record(out, 2)[0] == 0 && !memcmp(record(out, 3), record(old, 3), 5));

    /* Replacement failure leaves the original file and all transferred items intact. */
    state(); size = fixture(old, 0, 0); write_fixture(old, size);
    char temporary[1200]; snprintf(temporary, sizeof temporary, "%s/cache.tmp.new", folder);
    assert(!mkdir(temporary, 0700));
    ww(SS, SP + 4, 0x1950); key_hook();
    assert(g_rb(G_MEMBER(0) + G_M_NITEMS) == 1 && rb(G_FAR2E38, G_ITEM(0, 0) + 4) == 1);
    assert(!g_rw(GV_cache_n) && (s16)rw(0x6100, 0x118) == -1 && redraws == 1 && notices == 1);
    assert(read_fixture(got) == size && !memcmp(old, got, size));
    assert(!rmdir(temporary));
    SP = 0x8000; ww(SS, SP + 4, 0x1950); key_hook();
    assert(!g_rb(G_MEMBER(0) + G_M_NITEMS) && rw(0x6100, 0x118) == 1);
    read_fixture(got); assert(got[1] == 1 && record(got, 1)[0] == 1 && record(got, 1)[4] == 1);
    /* Disabling the fix and original mode both delegate without host writes. */
    state(); enabled = 0; size = fixture(old, 0, 0); write_fixture(old, size);
    ww(SS, SP + 4, 0x1950); key_hook(); assert(original_calls == 1 && !notices);
    assert(read_fixture(got) == size && !memcmp(old, got, size));
    state(); original_mode = 1; ww(SS, SP + 4, 1); save_hook(); assert(original_calls == 1);
    assert(read_fixture(got) == size && !memcmp(old, got, size));
    state(); g_ww(GV_cache_n, CACHE_ENTRIES); g_ww(GV_cache_n_shown, CACHE_ENTRIES);
    wb(G_FAR2E38, CACHE_ITEMS + 4 * CACHE_ENTRIES, 0xab);
    ww(SS, SP + 4, 0x1950); key_hook();
    assert(!original_calls && notices == 1 && g_rb(G_MEMBER(0) + G_M_NITEMS) == 1);
    assert(rb(G_FAR2E38, CACHE_ITEMS + 4 * CACHE_ENTRIES) == 0xab);

    char path[1200]; dos_host_path("CACHE.TMP", path, sizeof path); unlink(path); rmdir(folder);
    free(old); free(out); free(got); free(M);
    puts("cache file regression checks: PASS");
    return 0;
}
