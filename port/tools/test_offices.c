/* Office cooldown regression checks against the game's emulated event table.
 * From the repository root:
 * gcc -ffunction-sections -fdata-sections -Wl,--gc-sections -o build/test_offices \
 *     port/tools/test_offices.c port/mods/offices.c port/mods/gamemodel.c && build/test_offices
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "../mods/offices.h"
#include "../runtime/game.h"

u8 *M;
CPU R;
u16 LOADSEG;
int assist_best_leader;

enum { TOWN = 84, DATA_SEG = 0x5000, LOC_OFF = 0x100 };

static void date(u32 event, u16 field, int hour, int day, int month, int year)
{
    g_ev_set(event, field, hour);
    g_ev_set(event, field + 2, day);
    g_ev_set(event, field + 4, month);
    g_ev_set(event, field + 6, year);
}

static u32 cooldown(int slot, int kind, int giver)
{
    u16 ptr = G_EVENTS + slot * 4;
    u16 off = 0x200 + slot * 0x30;
    ww(G_FAR2E38, ptr, off);
    ww(G_FAR2E38, ptr + 2, DATA_SEG);
    u32 event = g_event(slot);
    g_ev_set(event, G_EV_KIND, kind);
    g_ev_set(event, G_EV_TYPE0, giver);
    g_ev_set(event, G_EV_LOC, TOWN);
    return event;
}

int main(void)
{
    M = calloc(1, MEM_SIZE);
    u8 *before = malloc(MEM_SIZE);
    assert(M && before);
    ww(G_FAR2E38, 0x10be + TOWN * 4, LOC_OFF);
    ww(G_FAR2E38, 0x10be + TOWN * 4 + 2, DATA_SEG);
    ww(DATA_SEG, LOC_OFF + 0x12, 20);                   /* steady local reputation */
    g_ww(G_CUR_HOUR, 12);
    g_ww(G_CUR_DAY, 30);
    g_ww(G_CUR_MONTH, 11);
    g_ww(G_CUR_YEAR, 1400);

    u32 bank = cooldown(0, 7, 8);
    date(bank, G_EV_START, 14, 29, 11, 1400);
    date(bank, G_EV_END, 14, 1, 0, 1401);
    u32 hall = cooldown(1, 0x3c, 0);
    date(hall, G_EV_START, 11, 27, 11, 1400);
    date(hall, G_EV_END, 11, 3, 0, 1401);
    u32 later = cooldown(2, 7, 6);
    date(later, G_EV_START, 12, 31, 11, 1400);
    date(later, G_EV_END, 12, 3, 0, 1401);

    memcpy(before, M, MEM_SIZE);
    OfficeStatus status;
    assert(office_status(TOWN, 8, 0, &status));
    assert(status.state == OFS_SHUT && status.chance == -1);
    assert(status.hour == 14 && status.day == 1 && status.month == 0 && status.year == 1401);
    assert(office_status(TOWN, 10, 1, &status));
    assert(status.state == OFS_SHUT && status.chance == 1);
    assert(status.hour == 11 && status.day == 3 && status.month == 0 && status.year == 1401);
    assert(office_status(TOWN, 6, 0, &status));
    assert(status.state == OFS_ASK && status.chance == 21);
    assert(memcmp(M, before, MEM_SIZE) == 0);

    /* The council checks reputation first, and a reward grants an audience during the cooldown. */
    ww(DATA_SEG, LOC_OFF + 0x12, (u16)-40);
    assert(office_status(TOWN, 10, 1, &status));
    assert(status.state == OFS_BARRED && status.chance == -1);
    ww(DATA_SEG, LOC_OFF + 0x12, 20);
    u32 reward = cooldown(3, 0x24, 0);
    g_ev_set(reward, G_EV_OWNER, 10);
    g_ev_set(reward, G_EV_TAG, 3);
    date(reward, G_EV_START, 11, 30, 11, 1400);
    assert(office_status(TOWN, 10, 1, &status));
    assert(status.state == OFS_REWARD);
    assert(office_status(TOWN, 10, 0, &status));
    assert(status.state == OFS_SHUT && status.chance == 1);

    free(before);
    free(M);
    return 0;
}
