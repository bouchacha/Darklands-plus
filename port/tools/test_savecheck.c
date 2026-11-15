/* test_savecheck.c - synthetic save validation checks, including bounded decompression.
 * gcc -Wall -Wextra -fsanitize=address,undefined -g -o /tmp/test_savecheck \
 *   port/tools/test_savecheck.c port/mods/savecheck.c port/third_party/blast/blast.c
 * /tmp/test_savecheck
 * Every fixture is generated here; no game data is required.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../mods/savecheck.h"

static void set_word(unsigned char *bytes, unsigned value)
{
    bytes[0] = (unsigned char)value;
    bytes[1] = (unsigned char)(value >> 8);
}

static void check(const unsigned char *data, size_t size, int battle, int valid)
{
    FILE *file = tmpfile();
    assert(file);
    assert(fwrite(data, 1, size, file) == size);
    const char *problem = battle ? savecheck_bsv(file) : savecheck_sav(file);
    if (!!problem == !!valid) fprintf(stderr, "%s file of %zu bytes: %s\n", battle ? "battle" : "save", size, problem ? problem : "accepted");
    assert(!!problem != !!valid);
    fclose(file);
}

static void saves(void)
{
    const size_t events_at = 0x189 + 0x22a;
    const size_t locations_at = events_at + 2;
    const size_t size = locations_at + 2 + 414 * 0x3a;
    unsigned char *data = calloc(1, size + 200);
    assert(data);
    set_word(data + 0x68, 1400);
    set_word(data + 0x6a, 4);
    set_word(data + 0x6c, 27);
    set_word(data + 0x6e, 9);
    set_word(data + 0xf1, 1);
    set_word(data + locations_at, 414);
    check(data, size, 0, 1);
    check(data, size + 200, 0, 1); /* an opaque cache tail */
    data[0xad] = 'D'; data[0xae] = '+'; /* the Darklands+ header extension */
    check(data, size, 0, 1);
    set_word(data + 0x6e, 0xffff); /* the original can write a negative hour */
    check(data, size, 0, 1);
    set_word(data + locations_at, 405); /* the loader reads its fixed block regardless */
    check(data, size, 0, 1);
    set_word(data + locations_at, 414);
    const size_t cuts[] = { 0, 96, 0x188, 512, events_at + 1, locations_at + 1, size - 1 };
    for (size_t i = 0; i < sizeof cuts / sizeof cuts[0]; i++) check(data, cuts[i], 0, 0);
    set_word(data + 0x6a, 12); check(data, size, 0, 0);
    set_word(data + 0x6a, 4);
    set_word(data + 0x7c, 414); check(data, size, 0, 0);
    set_word(data + 0x7c, 0xffff); check(data, size, 0, 1);
    set_word(data + events_at, 301); check(data, size, 0, 0);
    set_word(data + events_at, 0);
    set_word(data + locations_at, 415); check(data, size, 0, 0);
    free(data);
}

/* DCL's uncoded literal format. Each byte has a zero tag bit, then its eight bits.
 * Length symbol 15 followed by extra 255 is the end marker (length 519).
 * This deliberately simple encoder supplies independent, generated decoder fixtures. */
static void bits(unsigned char *out, size_t *position, unsigned value, unsigned count)
{
    for (unsigned i = 0; i < count; i++, (*position)++)
        if (value & (1u << i)) out[*position / 8] |= 1u << (*position % 8);
}

static unsigned literals(unsigned char *out, const unsigned char *data, unsigned size)
{
    size_t position = 16;
    memset(out, 0, 4 + (size * 9 + 7) / 8);
    out[1] = 6;
    for (unsigned i = 0; i < size; i++) {
        bits(out, &position, 0, 1);
        bits(out, &position, data[i], 8);
    }
    bits(out, &position, 1, 1);
    bits(out, &position, 0, 7);
    bits(out, &position, 255, 8);
    return (unsigned)((position + 7) / 8);
}

static void battles(void)
{
    unsigned char *data = calloc(1, 100000), *raw = calloc(1, 70000);
    assert(data && raw);
    unsigned packed = literals(data + 0x2a, raw, 0x1978);
    set_word(data, packed);
    set_word(data + 34, 0xffff); set_word(data + 38, 0xffff);
    size_t size = 0x2a + packed;
    check(data, size, 1, 1);
    for (size_t cut = 0; cut < size; cut += 503) check(data, cut, 1, 0);
    check(data, size - 1, 1, 0);
    data[0x2a] = 7; check(data, size, 1, 0); data[0x2a] = 0;
    raw[0x16e4] = 17;
    literals(data + 0x2a, raw, 0x1978); check(data, size, 1, 0);
    raw[0x16e4] = 1;
    literals(data + 0x2a, raw, 0x1978);
    memset(raw, 0, 70000);
    unsigned level = literals(data + size, raw, 0x33fc + 2);
    set_word(data + 2, level);
    check(data, size + level, 1, 1);
    raw[0x33fc] = 1; /* says a group exists, but its 30 bytes are absent */
    literals(data + size, raw, 0x33fc + 2); check(data, size + level, 1, 0);
    raw[0x33fc] = 0;
    literals(data + size, raw, 0x33fc + 2);
    size += level;
    raw[0] = 1; raw[1] = 2; raw[2] = 3;
    unsigned work = literals(data + size, raw, 3);
    set_word(data + 38, 3); set_word(data + 40, work);
    check(data, size + work, 1, 1);
    check(data, size + work - 1, 1, 0);
    set_word(data + 38, 4); check(data, size + work, 1, 0);
    free(raw); free(data);
}

int main(void)
{
    saves();
    battles();
    puts("save validation checks: PASS");
    return 0;
}
