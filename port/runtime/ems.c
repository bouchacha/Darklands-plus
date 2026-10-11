/* ems.c - LIM EMS 4.0 expanded memory (int 67h), which Darklands requires.
 * Page frame at E000:0000 (four 16 KB physical pages). Logical pages live in host memory;
 * mapping copies a page into the frame (and the previous contents back out). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "hle.h"

#define EMS_FRAME 0xE000
#define EMS_PAGES 512                 /* 8 MB */
#define PG 16384
static u8 *store;                     /* EMS_PAGES * PG */
static int page_owner[EMS_PAGES];     /* handle owning the page, -1 free */
typedef struct { int used; int npages; int pages[EMS_PAGES]; char name[8]; } Handle;
static Handle handles[64];
static int mapped[4] = {-1, -1, -1, -1};   /* physical page -> global logical page */
static int saved_maps[64][4];
static int has_saved[64];
int ems_trace = -1;

static void unmap(int phys)
{
    if (mapped[phys] >= 0) memcpy(store + (size_t)mapped[phys] * PG, M + EMS_FRAME * 16 + phys * PG, PG);
    mapped[phys] = -1;
}
static void map_global(int phys, int g)
{
    if (mapped[phys] == g) return;
    unmap(phys);
    if (g >= 0) {
        /* the same page mapped elsewhere: flush that copy first */
        for (int p = 0; p < 4; p++) if (p != phys && mapped[p] == g) unmap(p);
        memcpy(M + EMS_FRAME * 16 + phys * PG, store + (size_t)g * PG, PG);
    }
    mapped[phys] = g;
}

void ems_init(void)
{
    store = calloc(EMS_PAGES, PG);
    for (int i = 0; i < EMS_PAGES; i++) page_owner[i] = -1;
    handles[0].used = 1;               /* system handle, 0 pages */
}

static int free_pages(void) { int n = 0; for (int i = 0; i < EMS_PAGES; i++) if (page_owner[i] < 0) n++; return n; }

static int alloc_into(int h, int n)
{
    if (n > free_pages()) return 0;
    for (int i = 0; i < EMS_PAGES && n; i++)
        if (page_owner[i] < 0) { page_owner[i] = h; handles[h].pages[handles[h].npages++] = i; n--; }
    return 1;
}

static int map_page(int h, int logical, int phys)
{
    if (phys < 0 || phys > 3) return 0x8b;
    if (h < 0 || h >= 64 || !handles[h].used) return 0x83;
    if (logical == 0xffff) { unmap(phys); return 0; }
    if (logical >= handles[h].npages) return 0x8a;
    map_global(phys, handles[h].pages[logical]);
    return 0;
}

static void sync_out(void) { for (int p = 0; p < 4; p++) if (mapped[p] >= 0) memcpy(store + (size_t)mapped[p] * PG, M + EMS_FRAME * 16 + p * PG, PG); }
static void sync_in(void) { for (int p = 0; p < 4; p++) if (mapped[p] >= 0) memcpy(M + EMS_FRAME * 16 + p * PG, store + (size_t)mapped[p] * PG, PG); }

/* address of byte i of a region descriptor (type, handle, offset, seg/page) */
static u8 *region_ptr(int type, int h, u16 off, u16 segpage, u32 i, int *err)
{
    if (type == 0) { u32 a = (u32)segpage * 16 + off + i; if (a >= MEM_SIZE) { *err = 0xa2; return 0; } return M + a; }
    if (h < 0 || h >= 64 || !handles[h].used) { *err = 0x83; return 0; }
    u32 lo = (u32)off + i, pg = segpage + lo / PG;
    if (pg >= (u32)handles[h].npages) { *err = 0x93; return 0; }
    return store + (size_t)handles[h].pages[pg] * PG + lo % PG;
}

static int move_region(int xchg)
{
    u32 len = rw(DS, SI) | ((u32)rw(DS, (u16)(SI + 2)) << 16);
    int st = rb(DS, (u16)(SI + 4)), sh = rw(DS, (u16)(SI + 5)); u16 so = rw(DS, (u16)(SI + 7)), ss = rw(DS, (u16)(SI + 9));
    int dt = rb(DS, (u16)(SI + 11)), dh = rw(DS, (u16)(SI + 12)); u16 dof = rw(DS, (u16)(SI + 14)), ds = rw(DS, (u16)(SI + 16));
    int err = 0;
    sync_out();
    for (u32 i = 0; i < len && !err; i++) {
        u8 *a = region_ptr(st, sh, so, ss, i, &err), *b = err ? 0 : region_ptr(dt, dh, dof, ds, i, &err);
        if (err) break;
        if (xchg) { u8 t = *a; *a = *b; *b = t; } else *b = *a;
    }
    sync_in();
    if (!err && len > 0 && st == 0 && dt == 0) err = 0;  /* overlap allowed, same as DOS EMM */
    return err;
}

void ems_int67(void)
{
    int h = DX;
    if (ems_trace < 0) ems_trace = getenv("DL_TRACE_EMS") != 0;
    if (ems_trace) { int dl_in_irq(void); u32 plat_ticks_ms(void); fprintf(stderr, "int67 %u ax=%04x bx=%04x dx=%04x%s map %d %d %d %d\n", plat_ticks_ms(), AX, BX, DX, dl_in_irq() ? " IRQ" : "", mapped[0], mapped[1], mapped[2], mapped[3]); }
    switch (AH) {
    case 0x40: AH = 0; break;
    case 0x41: AH = 0; BX = EMS_FRAME; break;
    case 0x42: AH = 0; BX = (u16)free_pages(); DX = EMS_PAGES; break;
    case 0x5a: if (AL > 1) { AH = 0x8f; break; } /* fall through: standard / raw pages */
    case 0x43: {
        int k; for (k = 1; k < 64 && handles[k].used; k++) {}
        if (k == 64) { AH = 0x85; break; }
        if (BX > free_pages()) { AH = 0x88; break; }
        handles[k].used = 1; handles[k].npages = 0; memset(handles[k].name, 0, 8);
        alloc_into(k, BX); DX = (u16)k; AH = 0; break; }
    case 0x44: AH = (u8)map_page(h, BX, AL); break;
    case 0x45:
        if (h <= 0 || h >= 64 || !handles[h].used) { AH = 0x83; break; }
        for (int p = 0; p < 4; p++) if (mapped[p] >= 0 && page_owner[mapped[p]] == h) mapped[p] = -1;
        for (int i = 0; i < handles[h].npages; i++) page_owner[handles[h].pages[i]] = -1;
        handles[h].used = 0; handles[h].npages = 0; AH = 0; break;
    case 0x46: AH = 0; AL = 0x40; break;
    case 0x47: if (h < 64) { memcpy(saved_maps[h], mapped, sizeof mapped); has_saved[h] = 1; } AH = 0; break;
    case 0x48: if (h < 64 && has_saved[h]) { for (int p = 0; p < 4; p++) map_global(p, saved_maps[h][p]); has_saved[h] = 0; AH = 0; } else AH = 0x8e; break;
    case 0x4b: { int n = 0; for (int k = 0; k < 64; k++) n += handles[k].used; BX = (u16)n; AH = 0; break; }
    case 0x4c: if (h < 64 && handles[h].used) { BX = (u16)handles[h].npages; AH = 0; } else AH = 0x83; break;
    case 0x4d: { int n = 0; for (int k = 0; k < 64; k++) if (handles[k].used) {
            ww(ES, (u16)(DI + 4 * n), (u16)k); ww(ES, (u16)(DI + 4 * n + 2), (u16)handles[k].npages); n++; }
        BX = (u16)n; AH = 0; break; }
    case 0x4e:      /* get/set page map: 4 ints */
        if (AL == 0 || AL == 2) for (int p = 0; p < 4; p++) ww(ES, (u16)(DI + 2 * p), (u16)mapped[p]);
        if (AL == 1 || AL == 2) for (int p = 0; p < 4; p++) map_global(p, (s16)rw(DS, (u16)(SI + 2 * p)));
        if (AL == 3) AL = 8;
        AH = 0; break;
    case 0x50: {    /* map multiple: AL=0 physical page numbers, 1 segments */
        int r = 0;
        for (int i = 0; i < CX && !r; i++) {
            u16 lg = rw(DS, (u16)(SI + 4 * i)), ph = rw(DS, (u16)(SI + 4 * i + 2));
            if (AL == 1) ph = (u16)((ph - EMS_FRAME) / 0x400);
            r = map_page(h, lg, ph);
        }
        AH = (u8)r; break; }
    case 0x51:
        if (h < 64 && handles[h].used) {
            int n = BX;
            while (handles[h].npages > n) page_owner[handles[h].pages[--handles[h].npages]] = -1;
            if (n > handles[h].npages && !alloc_into(h, n - handles[h].npages)) { AH = 0x88; break; }
            AH = 0;
        } else AH = 0x83;
        break;
    case 0x53:
        if (AL == 0) { for (int i = 0; i < 8; i++) wb(ES, (u16)(DI + i), (u8)handles[h].name[i]); AH = 0; }
        else { for (int i = 0; i < 8; i++) handles[h].name[i] = (char)rb(DS, (u16)(SI + i)); AH = 0; }
        break;
    case 0x57: AH = (u8)move_region(AL == 1); break;
    case 0x58:
        if (AL == 0) for (int p = 0; p < 4; p++) { ww(ES, (u16)(DI + 4 * p), (u16)(EMS_FRAME + p * 0x400)); ww(ES, (u16)(DI + 4 * p + 2), (u16)p); }
        CX = 4; AH = 0; break;
    case 0x59: if (AL == 1) { BX = (u16)free_pages(); DX = EMS_PAGES; } AH = 0; break;
    default:
        fprintf(stderr, "int 67h: unimplemented AX=%04x\n", AX);
        AH = 0x84;
    }
}
