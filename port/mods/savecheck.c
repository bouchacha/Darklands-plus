/* savecheck.c - check saved games before the game's load discards the running session.
 * The layouts follow the readers s037 1c7d:0d24 and s003 0e76:0ce8. Only bytes read by
 * those readers are required. The stash tail and header extensions stay opaque.
 * SAVES/DEFAULT is a new-game template read by a different routine, not a loadable save.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "savecheck.h"
#include "../third_party/blast/blast.h"

static unsigned word(const unsigned char *bytes)
{
    return bytes[0] | (unsigned)bytes[1] << 8;
}

static int signed_word(const unsigned char *bytes)
{
    unsigned value = word(bytes);
    return value < 0x8000 ? (int)value : (int)value - 0x10000;
}

const char *savecheck_sav(FILE *file)
{
    unsigned char header[0x189], count[2];
    if (fseek(file, 0, SEEK_END)) return "the save could not be read";
    long size = ftell(file);
    if (size < (long)sizeof header || fseek(file, 0, SEEK_SET)
        || fread(header, 1, sizeof header, file) != sizeof header)
        return "the save header is incomplete";

    int month = signed_word(header + 0x6a), day = signed_word(header + 0x6c);
    int hour = signed_word(header + 0x6e), place = signed_word(header + 0x7c);
    /* A negative hour can be written by the original's subtraction of hours. */
    if (month < 0 || month > 11 || day < 0 || day > 31 || hour > 23)
        return "the saved date is outside the game's calendar";
    if (place < -1 || place >= 414) return "the saved location is outside the world";

    long at = (long)sizeof header + (long)word(header + 0xf1) * 0x22a;
    if (at > size - 2 || fseek(file, at, SEEK_SET) || fread(count, 1, 2, file) != 2)
        return "the character records are incomplete";
    unsigned events = word(count);
    if (events > 300) return "the event count exceeds the game's table";
    at += 2 + (long)events * 0x30;
    if (at > size - 2 || fseek(file, at, SEEK_SET) || fread(count, 1, 2, file) != 2)
        return "the event records are incomplete";
    /* The load always reads 414 records, even when the count word says otherwise. */
    if (word(count) > 414) return "the location count exceeds the game's table";
    if (at + 2 + 414L * 0x3a > size) return "the location records are incomplete";
    return NULL;
}

typedef struct {
    unsigned char *data;
    unsigned size, capacity;
} Output;

static unsigned no_more_input(void *unused, unsigned char **bytes)
{
    (void)unused; (void)bytes;
    return 0;
}

static int take_output(void *state, unsigned char *bytes, unsigned size)
{
    Output *out = state;
    if (size > out->capacity - out->size) return 1;
    memcpy(out->data + out->size, bytes, size);
    out->size += size;
    return 0;
}

static int block(FILE *file, unsigned packed, unsigned char *input, Output *out)
{
    if (packed < 2 || fread(input, 1, packed, file) != packed) return 0;
    unsigned char *next = input;
    unsigned left = packed;
    out->size = 0;
    return blast(no_more_input, NULL, take_output, out, &left, &next) == 0;
}

static int level_complete(const Output *out)
{
    if (out->size < 0x33fc + 2) return 0;
    /* The original reads both counts as signed chars before multiplying their sizes. */
    if (out->data[0x33fc] > 127) return 0;
    unsigned at = 0x33fc + 1 + (unsigned)out->data[0x33fc] * 30;
    if (at >= out->size) return 0;
    if (out->data[at] > 127) return 0;
    return at + 1 + (unsigned)out->data[at] * 0x122 <= out->size;
}

const char *savecheck_bsv(FILE *file)
{
    unsigned char header[0x2a];
    if (fseek(file, 0, SEEK_SET) || fread(header, 1, sizeof header, file) != sizeof header)
        return "the battle-file header is incomplete";
    unsigned char *input = malloc(0xffff), *output = malloc(0xffff);
    if (!input || !output) {
        free(input); free(output);
        return "there is not enough memory to check the battle file";
    }
    Output out = { output, 0, 0xffff };
    const char *problem = NULL;
    if (!block(file, word(header), input, &out) || out.size != 0x1978) {
        problem = "the battle variables are incomplete or corrupt";
        goto done;
    }
    unsigned levels = output[0x16e4];
    if (levels > 16) {
        problem = "the battle file has too many floors";
        goto done;
    }
    for (unsigned i = 0; i < levels; i++) {
        if (!block(file, word(header + 2 + 2 * i), input, &out) || !level_complete(&out)) {
            problem = "a saved battle floor is incomplete or corrupt";
            goto done;
        }
    }
    /* The work files are stored terrain first, then loot, opposite to their header order. */
    const unsigned entries[] = { 19, 17 };
    for (unsigned i = 0; i < 2; i++) {
        unsigned raw = word(header + 2 * entries[i]);
        if (raw == 0 || raw == 0xffff) continue;
        if (!block(file, word(header + 2 * (entries[i] + 1)), input, &out) || out.size < raw) {
            problem = "a battle work file is incomplete or corrupt";
            goto done;
        }
    }
done:
    free(input); free(output);
    return problem;
}
