/* ovl.c - RTLink/Plus overlay sections: loaded into memory on demand, as RTLink does.
 * The code itself is native (recompiled); loading the section bytes keeps code-segment data,
 * relocated segment values and "which sibling overlay is present" faithful. */
#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "dispatch.h"
#include "hle.h"

typedef struct { u16 load_seg, file_para_lo, file_para_hi_flags, mem_paras, nrelocs, parent, sect_id, file_paras; } Sect;
static Sect sect[256];
static int nsect;
int ovl_loaded[256];
static FILE *exef;
static u32 sect_file_off[256];

static u16 le16(const u8 *p) { return (u16)(p[0] | (p[1] << 8)); }

void ovl_init(const char *exe_path)
{
    exef = fopen(exe_path, "rb");
    if (!exef) fatal("cannot open %s", exe_path);
    u8 h[32];
    fread(h, 1, 32, exef);
    u32 hdr = le16(h + 8) * 16u;
    /* RTLink section table: manager segment 09c0, count word at +0b74, table at +0b7a */
    u32 tab = hdr + 0x9c00 + 0xb7a, cnt = hdr + 0x9c00 + 0xb74;
    u8 b[2];
    fseek(exef, cnt, SEEK_SET); fread(b, 1, 2, exef);
    nsect = le16(b);
    if (nsect <= 0 || nsect > 256) fatal("bad section count %d", nsect);
    for (int i = 0; i < nsect; i++) {
        u8 e[18];
        fseek(exef, tab + 18 * i, SEEK_SET); fread(e, 1, 18, exef);
        Sect *s = &sect[i];
        s->load_seg = le16(e); s->file_para_lo = le16(e + 4); s->file_para_hi_flags = le16(e + 6);
        s->mem_paras = le16(e + 8); s->nrelocs = le16(e + 10); s->parent = le16(e + 12);
        s->sect_id = le16(e + 14); s->file_paras = le16(e + 16);
        sect_file_off[i] = (s->file_para_lo | ((u32)(s->file_para_hi_flags & 0xff) << 16)) * 16u;
    }
}

static void load(int i)
{
    Sect *s = &sect[i];
    static int tr = -1; if (tr < 0) tr = getenv("DL_TRACE_OVL") != 0;
    if (tr) fprintf(stderr, "ovl: load s%03d at %04x\n", i, s->load_seg);
    if (s->parent != 0xffff && s->parent < nsect && !ovl_loaded[s->parent]) load(s->parent);
    u32 rbytes = s->nrelocs * 4u;
    u32 pad = (16 - rbytes % 16) % 16;
    u8 *rel = malloc(rbytes + 1);
    fseek(exef, sect_file_off[i], SEEK_SET);
    fread(rel, 1, rbytes, exef);
    fseek(exef, sect_file_off[i] + rbytes + pad, SEEK_SET);
    u32 dst = ((u32)LOADSEG + s->load_seg) * 16u;
    u32 fsz = s->file_paras * 16u, msz = s->mem_paras * 16u;
    if (dst + msz > MEM_SIZE) fatal("overlay %d out of memory", i);
    memset(M + dst, 0, msz);
    fread(M + dst, 1, fsz, exef);
    for (u32 k = 0; k < s->nrelocs; k++) {
        u16 off = le16(rel + 4 * k), seg = le16(rel + 4 * k + 2);
        u32 a = ((u32)LOADSEG + seg) * 16u + off;
        u16 v = (u16)(M[a] | (M[a + 1] << 8)) + LOADSEG;
        M[a] = (u8)v; M[a + 1] = (u8)(v >> 8);
    }
    free(rel);
    /* sections overlapping this one are gone */
    for (int j = 0; j < nsect; j++) {
        if (j == i || !ovl_loaded[j]) continue;
        u32 a0 = sect[j].load_seg, a1 = a0 + sect[j].mem_paras;
        u32 b0 = s->load_seg, b1 = b0 + s->mem_paras;
        if (a0 < b1 && b0 < a1) ovl_loaded[j] = 0;
    }
    ovl_loaded[i] = 1;
    { void watch_check(const char *); watch_check("overlay load"); }
}

void ovl_enter(int i)
{
    if (i < 0 || i >= nsect || ovl_loaded[i]) return;
    load(i);
}

/* RTLink's own start-up preloads sections; the native start-up does it here */
void ovl_preload(void)
{
    for (int i = 0; i < nsect; i++)
        if ((sect[i].file_para_hi_flags >> 8) & 1) load(i);
}

void ovl_dump(void)
{
    fprintf(stderr, "  loaded overlay sections:");
    for (int i = 0; i < nsect; i++) if (ovl_loaded[i]) fprintf(stderr, " s%03d@%04x", i, sect[i].load_seg);
    fprintf(stderr, "\n");
}

/* the loaded overlay section containing a link-time linear address (-1: root / none) */
int ovl_section_at(u32 lin)
{
    int best = -1;
    for (int i = 0; i < nsect; i++) {
        if (!ovl_loaded[i] || i == 136) continue;
        u32 a = sect[i].load_seg * 16u, b = a + sect[i].mem_paras * 16u;
        if (lin >= a && lin < b) best = i;
    }
    return best;
}

/* the overlay area (all sections except the preloaded data section): for snapshots */
void ovl_area(u32 *lin_start, u32 *lin_end)
{
    u32 a = 0xffffffff, b = 0;
    for (int i = 0; i < nsect; i++) {
        if ((sect[i].file_para_hi_flags >> 8) & 1) continue;      /* preloaded (data) */
        u32 s0 = sect[i].load_seg * 16u, s1 = s0 + sect[i].mem_paras * 16u;
        if (s0 < a) a = s0;
        if (s1 > b) b = s1;
    }
    *lin_start = a; *lin_end = b;
}
int ovl_count(void) { return nsect; }
