/* cachefile.c - keep each inn's stored items when the cache file is rewritten.
 * s035 1462:1468 reads the first offset through a signed byte, changes the header's
 * length, then copies the other stores from consecutive memory. Read each store by
 * its full offset instead, keep its handle, and replace the file after a whole write.
 * Transfers are committed before the screen continues, so a write refused for space
 * or an I/O error can return the item to where it was. Shared-store merges write the
 * destination and emptied source together. Findings B-85e0e4 and B-3de49c are in
 * Darklands Restoration's original-bug register.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/hooks.h"
#include "../runtime/hle.h"
#include "fixes.h"
#include "nscreen.h"
#include "cachefile.h"

#define CACHE_LIMIT 65535
#define CACHE_ENTRIES 100
#define CACHE_HANDLES 98
#define CACHE_ITEMS 0x37f4
#define LOCATION_CACHE 0x18

void dos_host_path_write(const char *, char *, size_t);
void ui_notice(const char *);

static unsigned read_word(const u8 *data)
{
    return data[0] | (unsigned)data[1] << 8;
}
static void write_word(u8 *data, unsigned value)
{
    data[0] = (u8)value; data[1] = (u8)(value >> 8);
}

/* Only the table and its records are interpreted. Unused gaps or old trailing bytes
 * are discarded, while empty stores and every handle retain their position. */
static const char *cache_rewrite(const u8 *old, size_t old_size, int handle, int clear,
                                const u8 *items, unsigned count, u8 *out, size_t *out_size)
{
    if (old_size < 2 || count > CACHE_ENTRIES) return "the store file is incomplete";
    unsigned stores = old[1], offsets[256], lengths[256];
    if (old_size < 2 + 2 * stores) return "the store offset table is incomplete";
    for (unsigned h = 1; h <= stores; h++) {
        unsigned at = read_word(old + 2 * h);
        if (at < 2 + 2 * stores || at >= old_size) return "a store has an invalid offset";
        unsigned size = 1 + 4 * (unsigned)old[at];
        if (size > old_size - at) return "a stored item list is incomplete";
        offsets[h] = at; lengths[h] = size;
    }
    if (handle == -1) {
        if (stores >= CACHE_HANDLES) return "the file already holds 98 stores";
        handle = (int)++stores;
    } else if (handle < 1 || handle > (int)stores) return "this inn has no valid store handle";
    if (clear && (clear < 1 || clear > (int)stores || clear == handle))
        return "the stores to combine are invalid";

    /* The standard 198-byte header has room for 98 offset words. Keep any larger
     * legacy table readable, but never append past the original table's capacity. */
    unsigned header = 2 + 2 * stores;
    if (header < 198) header = 198;
    size_t total = header;
    for (unsigned h = 1; h <= stores; h++) {
        unsigned size = h == (unsigned)handle ? 1 + 4 * count : h == (unsigned)clear ? 1 : lengths[h];
        if (size > CACHE_LIMIT - total) return "the store file has no room for another item";
        total += size;
    }
    memset(out, 0, header);
    out[0] = (u8)(header / 2 > 255 ? 255 : header / 2);
    out[1] = (u8)stores;
    size_t at = header;
    for (unsigned h = 1; h <= stores; h++) {
        write_word(out + 2 * h, (unsigned)at);
        if (h == (unsigned)handle) {
            out[at++] = (u8)count;
            memcpy(out + at, items, count * 4); at += count * 4;
        } else if (h == (unsigned)clear) out[at++] = 0;
        else { memcpy(out + at, old + offsets[h], lengths[h]); at += lengths[h]; }
    }
    *out_size = at;
    return NULL;
}

static int replace_file(const char *temporary, const char *path)
{
#ifdef _WIN32
    return MoveFileExA(temporary, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    return rename(temporary, path) == 0;
#endif
}

/* The working table and location handle change only after the file is replaced. */
static const char *save_cache(int handle, int clear)
{
    int current = (s16)g_rw(G_CUR_LOC);
    u32 location = current >= 0 && current < G_NLOC ? g_location(current) : 0;
    if (handle == -1 && !location) return "the inn has no location";
    unsigned count = g_rw(GV_cache_n_shown);
    if (count > CACHE_ENTRIES) return "the store exceeds its 100-entry working table";
    u8 *old = malloc(CACHE_LIMIT), *out = malloc(CACHE_LIMIT);
    if (!old || !out) { free(old); free(out); return "there is not enough memory"; }
    const char *problem = NULL;
    char path[1200], temporary[1220];
    dos_host_path_read("CACHE.TMP", path, sizeof path);
    FILE *file = fopen(path, "rb");
    size_t old_size = file ? fread(old, 1, CACHE_LIMIT, file) : 0;
    if (!file || ferror(file)) problem = "the store file could not be read";
    if (file) fclose(file);
    size_t size = 0;
    if (!problem) problem = cache_rewrite(old, old_size, handle, clear,
        M + LIN(G_FAR2E38, CACHE_ITEMS), count, out, &size);
    if (!problem) {
        dos_host_path_write("CACHE.TMP", path, sizeof path);
        snprintf(temporary, sizeof temporary, "%s.new", path);
        file = fopen(temporary, "wb");
        int created = file != NULL;
        int ok = file && fwrite(out, 1, size, file) == size;
        if (file && fclose(file)) ok = 0;
        if (ok) ok = replace_file(temporary, path);
        if (!ok) { if (created) remove(temporary); problem = "the store file could not be written"; }
        else if (handle == -1)
            ww((u16)(location >> 16), (u16)((u16)location + LOCATION_CACHE), out[1]);
    }
    free(old); free(out);
    return problem;
}

static void tell_failure(const char *problem)
{
    char text[180]; snprintf(text, sizeof text, "The items stay where they were because %s.", problem);
    fprintf(stderr, "cachefile: %s\n", text);
    ui_notice(text);
}

static void save_hook(void)
{
    if (!fix_is_on("cachefile")) { hook_call_original(); return; }
    const char *problem = save_cache((s16)rw(SS, (u16)(SP + 4)), 0);
    if (problem) tell_failure(problem);
    AX = problem ? 0xffff : 0;
    hook_return_far(0);
}

int cachefile_merge(int destination, int source)
{
    if (!fix_is_on("cachefile")) return 0;
    const char *problem = save_cache(destination, source);
    if (problem) { tell_failure(problem); return -1; }
    return 1;
}

/* G/P can alter equipped items as well as stack counts. Keep their complete records,
 * inventories, working store and cursors until the write succeeds. No hours pass here. */
typedef struct {
    u8 members[5 * 0x80], inventory[5 * 0x180], items[4 * CACHE_ENTRIES], cursors[60];
    u16 count, shown;
} Transfer;

/* Even with the separate stack-limit fix disabled, a new row cannot be written
 * past the working table into the character-creation data that follows it. */
static int put_needs_extra_row(void)
{
    int member = (s16)g_rw(GV_cache_who);
    if (member < 0 || member >= 5 || g_rw((u16)(GV_cache_side + 2 * member))) return 0;
    int index = (s16)g_rw((u16)(GV_cache_inv_top + 2 * member))
        + (s16)g_rw((u16)(GV_cache_inv_sel + 2 * member));
    if (index < 0 || index >= 64 || index >= g_rb((u16)(G_MEMBER(member) + G_M_NITEMS))) return 0;
    u16 item = G_ITEM(member, index);
    unsigned id = rw(G_FAR2E38, item) & 0xfff, quality = rb(G_FAR2E38, (u16)(item + 3));
    for (unsigned row = 0; row < CACHE_ENTRIES; row++) {
        u16 entry = (u16)(CACHE_ITEMS + 4 * row);
        if (rw(G_FAR2E38, entry) == id && rb(G_FAR2E38, (u16)(entry + 2)) == quality) return 0;
    }
    return 1;
}

static void key_hook(void)
{
    int key = rw(SS, (u16)(SP + 4));
    int transfer = key == 0x1950 || key == 0x1970 || key == 0x2247 || key == 0x2267;
    if (!transfer || !fix_is_on("cachefile")) { hook_call_original(); return; }
    if (g_rw(GV_cache_n) > CACHE_ENTRIES || g_rw(GV_cache_n_shown) > CACHE_ENTRIES) {
        tell_failure("the store exceeds its 100-entry working table"); hook_return_far(0); return;
    }
    if ((key == 0x1950 || key == 0x1970) && g_rw(GV_cache_n) == CACHE_ENTRIES && put_needs_extra_row()) {
        tell_failure("the store already holds 100 kinds of items"); hook_return_far(0); return;
    }
    int current = (s16)g_rw(G_CUR_LOC);
    u32 location = current >= 0 && current < G_NLOC ? g_location(current) : 0;
    if (!location) { tell_failure("the inn has no location"); hook_return_far(0); return; }
    Transfer before;
    memcpy(before.members, M + LIN(G_DGROUP, CB_REC(0)), sizeof before.members);
    memcpy(before.inventory, M + LIN(G_FAR2E38, G_INV), sizeof before.inventory);
    memcpy(before.items, M + LIN(G_FAR2E38, CACHE_ITEMS), sizeof before.items);
    memcpy(before.cursors, M + LIN(G_DGROUP, GV_cache_n_items), sizeof before.cursors);
    before.count = g_rw(GV_cache_n); before.shown = g_rw(GV_cache_n_shown);
    hook_call_original();
    if (ns_unwinding) return;
    if (before.count == g_rw(GV_cache_n)
        && !memcmp(before.items, M + LIN(G_FAR2E38, CACHE_ITEMS), sizeof before.items)) return;
    u16 ax = AX, dx = DX;
    const char *problem = save_cache((s16)rw((u16)(location >> 16), (u16)((u16)location + LOCATION_CACHE)), 0);
    if (problem) {
        memcpy(M + LIN(G_DGROUP, CB_REC(0)), before.members, sizeof before.members);
        memcpy(M + LIN(G_FAR2E38, G_INV), before.inventory, sizeof before.inventory);
        memcpy(M + LIN(G_FAR2E38, CACHE_ITEMS), before.items, sizeof before.items);
        memcpy(M + LIN(G_DGROUP, GV_cache_n_items), before.cursors, sizeof before.cursors);
        g_ww(GV_cache_n, before.count); g_ww(GV_cache_n_shown, before.shown);
        ns_call(GF_cache_redraw, 2, (s16)g_rw(GV_cache_who), (s16)g_rw(GV_cache_arg));
        if (ns_unwinding) return;
        tell_failure(problem);
    }
    AX = ax; DX = dx;
}

void cachefile_init(void)
{
    hook_install(GF_cache_save, save_hook);
    hook_install(GF_cache_key, key_hook);
}
