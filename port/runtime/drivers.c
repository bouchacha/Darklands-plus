/* drivers.c - the run-time loaded driver images (MGRAPHIC.EXE, MISC.EXE, *SOUND.DLC).
 * Their code is recompiled (port/recomp/recomp_drv.py); the game loads the image bytes
 * with DOS 4B03 as before (so tables and data inside them are real), and far calls into
 * the loaded image are routed to the recompiled functions. */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "cpu.h"
#include "dispatch.h"
#include "hle.h"

#ifndef DL_RTSPLIT
extern const DriverImage *DRIVERS[];
extern u16 *DRIVER_SEGS[];
#endif
static int active[16];
static u32 drv_size[16];

void driver_register(const char *filename, u16 seg, u32 size, unsigned hash)
{
    const char *b = filename;
    for (const char *p = filename; *p; p++) if (*p == '\\' || *p == '/' || *p == ':') b = p + 1;
    char name[16]; int n = 0;
    for (; *b && *b != '.' && n < 15; b++) name[n++] = (char)tolower((u8)*b);
    name[n] = 0;
    int pick = -1;
    for (int i = 0; DRIVERS[i] && pick < 0; i++) if (DRIVERS[i]->hash == hash) pick = i;      /* by content */
    for (int i = 0; DRIVERS[i] && pick < 0; i++) if (!strcmp(DRIVERS[i]->name, name)) pick = i;
    for (int i = pick; i >= 0; i = -1) {
        {
            *DRIVER_SEGS[i] = seg; active[i] = 1; drv_size[i] = size;
            for (int j = 0; DRIVERS[j]; j++)     /* overwritten images are gone */
                if (j != i && active[j]) { u32 a0 = (u32)*DRIVER_SEGS[j] << 4, a1 = a0 + drv_size[j], b0 = (u32)seg << 4, b1 = b0 + size;
                    if (a0 < b1 && b0 < a1) active[j] = 0; }
            fprintf(stderr, "driver %s (%s) loaded at %04x\n", DRIVERS[i]->name, filename, seg);
            { void ovl_area(u32 *, u32 *); u32 a0, a1; ovl_area(&a0, &a1); a0 += (u32)LOADSEG << 4; a1 += (u32)LOADSEG << 4;
              u32 d0 = (u32)seg << 4, d1 = d0 + size;
              if (d0 < a1 && a0 < d1) fprintf(stderr, "warning: driver %s at %04x lies in the overlay area (%05x-%05x)\n", DRIVERS[i]->name, seg, a0, a1); }
            { void musicpack_driver(const char *); musicpack_driver(filename); }
            return;
        }
    }
    fprintf(stderr, "warning: driver %s (%s) has no recompiled code\n", name, filename);
}

/* recompiled driver function at seg:off, or 0 */
void (*driver_find(u16 seg, u16 off))(void)
{
    u32 lin = LIN(seg, off);
    for (int i = 0; DRIVERS[i]; i++) {
        if (!active[i]) continue;
        u32 base = (u32)*DRIVER_SEGS[i] << 4;
        if (lin < base || lin >= base + drv_size[i]) continue;
        u32 o = lin - base;
        const DriverImage *d = DRIVERS[i];
        int lo = 0, hi = d->count - 1;
        while (lo <= hi) {
            int mid = (lo + hi) / 2;
            if (d->funcs[mid].lin == o) return d->funcs[mid].f;
            if (d->funcs[mid].lin < o) lo = mid + 1; else hi = mid - 1;
        }
        return 0;
    }
    return 0;
}

/* is seg:off the instruction after a call inside a loaded driver? */
int driver_retsite(u16 seg, u16 off)
{
    u32 lin = LIN(seg, off);
    for (int i = 0; DRIVERS[i]; i++) {
        if (!active[i]) continue;
        u32 base = (u32)*DRIVER_SEGS[i] << 4;
        if (lin < base || lin >= base + drv_size[i]) continue;
        u32 o = lin - base;
        const DriverImage *d = DRIVERS[i];
        int lo = 0, hi = d->nrets - 1;
        while (lo <= hi) {
            int mid = (lo + hi) / 2;
            if (d->rets[mid] == o) return 1;
            if (d->rets[mid] < o) lo = mid + 1; else hi = mid - 1;
        }
        return 0;
    }
    return 0;
}

/* the segment of a loaded driver whose image has sig at offset 0x10 (its name and date), or 0 */
u16 driver_seg_by_sig(const char *sig)
{
    size_t n = strlen(sig);
    for (int i = 0; DRIVERS[i]; i++) {
        if (!active[i]) continue;
        u32 base = (u32)*DRIVER_SEGS[i] << 4;
        if (!memcmp(M + base + 0x10, sig, n)) return *DRIVER_SEGS[i];
    }
    return 0;
}
